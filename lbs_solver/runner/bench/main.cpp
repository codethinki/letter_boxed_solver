#include "gpt_runner.hpp"
#include "percentile_reporter.hpp"
#include "puzzles.hpp"

#include "solver/solver.hpp"

#include <benchmark/benchmark.h>
#include <cth/io/file.hpp>

#include <algorithm>
#include <charconv>
#include <cstddef>
#include <format>
#include <optional>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace lbs::bench {

constexpr std::string_view WORD_LIST_PATH = "assets/words_easy.txt";

struct options {
    size_t random = 300;
    size_t constructed = 100;
    unsigned seed = 42;
};

struct word_list {
    std::vector<std::byte> data;
    std::string_view csv;
    std::vector<size_t> splits;
};

struct puzzle {
    std::string letters;
    std::string group;
    compression_index_t compressionIndex;
    words_data wordsData;
};

[[nodiscard]] std::optional<options> parse_options(int argc, char** argv) {
    options out{};

    for(int i = 1; i < argc; i++) {
        std::string_view const arg = argv[i];

        auto const parse = [arg](std::string_view flag, auto& value) {
            if(!arg.starts_with(flag))
                return false;

            auto const number = arg.substr(flag.size());
            auto const [ptr, ec] = std::from_chars(number.data(), number.data() + number.size(), value);
            return ec == std::errc{} && ptr == number.data() + number.size();
        };

        if(!parse("--random=", out.random) && !parse("--constructed=", out.constructed) && !parse("--seed=", out.seed)) {
            std::println(stderr, "unrecognized argument: {}", arg);
            return std::nullopt;
        }
    }
    return out;
}

[[nodiscard]] word_list load_word_list() {
    auto data = cth::io::file::read<std::byte>(WORD_LIST_PATH);
    std::string_view const csv{reinterpret_cast<char const*>(data.data()), data.size()};
    auto splits = split(csv, ',');
    return {std::move(data), csv, std::move(splits)};
}

[[nodiscard]] std::vector<std::string_view> word_views(word_list const& list) {
    std::vector<std::string_view> words{};
    words.reserve(list.splits.size() + 1);

    size_t begin = 0;
    for(auto const end : list.splits) {
        words.push_back(list.csv.substr(begin, end - begin));
        begin = end + 1;
    }
    words.push_back(list.csv.substr(begin));
    return words;
}

void register_benchmarks(std::span<puzzle const> puzzles, word_list const& list) {
    auto const csv = list.csv;
    auto const& splits = list.splits;

    benchmark::RegisterBenchmark("lbs/load/word_list/words_easy", [](benchmark::State& state) {
        for(auto _ : state) {
            auto data = cth::io::file::read<std::byte>(WORD_LIST_PATH);
            benchmark::DoNotOptimize(data);
        }
    });

    benchmark::RegisterBenchmark("lbs/split/word_list/words_easy", [csv](benchmark::State& state) {
        for(auto _ : state) {
            auto splits = split(csv, ',');
            benchmark::DoNotOptimize(splits);
        }
    });

    for(auto const& p : puzzles) {
        auto const name = [&p](std::string_view family) { return std::format("{}/{}/{}", family, p.group, p.letters); };

        benchmark::RegisterBenchmark(name("lbs/filter_and_hash"), [&p, csv, &splits](benchmark::State& state) {
            for(auto _ : state) {
                auto const index = gen_compression_index(p.letters);
                auto wordsData = filter_and_hash(index, csv, splits);
                benchmark::DoNotOptimize(wordsData);
            }
        });

        benchmark::RegisterBenchmark(name("lbs/solve"), [&p](benchmark::State& state) {
            for(auto _ : state) {
                auto solutions = solve(p.compressionIndex, p.wordsData.words, p.wordsData.wordHashes);
                benchmark::DoNotOptimize(solutions);
            }
        });

        benchmark::RegisterBenchmark(name("lbs/total"), [&p, csv, &splits](benchmark::State& state) {
            for(auto _ : state) {
                auto const index = gen_compression_index(p.letters);
                auto const& [words, wordHashes] = filter_and_hash(index, csv, splits);
                auto solutions = solve(index, words, wordHashes);
                benchmark::DoNotOptimize(solutions);
            }
        });

        benchmark::RegisterBenchmark(name("lbs/total_with_load"), [&p](benchmark::State& state) {
            for(auto _ : state) {
                auto const list = load_word_list();
                auto const index = gen_compression_index(p.letters);
                auto const& [words, wordHashes] = filter_and_hash(index, list.csv, list.splits);
                auto solutions = solve(index, words, wordHashes);
                benchmark::DoNotOptimize(solutions);
            }
        });

        benchmark::RegisterBenchmark(name("gpt/total_with_load"), [&p](benchmark::State& state) {
            LetterBoxedSolver solver{};
            for(auto _ : state) {
                auto solution = solver.run(p.letters, WORD_LIST_PATH);
                benchmark::DoNotOptimize(solution);
            }
        });
    }
}

}

int main(int argc, char** argv) {
    using namespace lbs::bench;

    std::string defaultMinTime = "--benchmark_min_time=0.01s";
    std::vector<char*> args(argv, argv + argc);
    args.insert(args.begin() + 1, defaultMinTime.data());

    auto argCount = static_cast<int>(args.size());
    benchmark::Initialize(&argCount, args.data());
    benchmark::SetDefaultTimeUnit(benchmark::kMicrosecond);

    auto const options = parse_options(argCount, args.data());
    if(!options)
        return 1;

    auto const list = load_word_list();
    auto const words = word_views(list);
    puzzle_generator generator{words, options->seed};

    std::vector<puzzle> puzzles{};
    LetterBoxedSolver gpt{};
    size_t disagreements = 0;
    size_t unsolvedConstructed = 0;

    auto const add = [&](std::string letters, bool constructed) {
        auto const index = lbs::gen_compression_index(letters);
        auto wordsData = lbs::filter_and_hash(index, list.csv, list.splits);

        bool const solvable = !lbs::solve(index, wordsData.words, wordsData.wordHashes).empty();
        disagreements += solvable == gpt.run(letters, WORD_LIST_PATH).empty();
        unsolvedConstructed += constructed && !solvable;

        std::string group = constructed ? "constructed" : solvable ? "random_solvable" : "random_unsolvable";
        puzzles.emplace_back(std::move(letters), std::move(group), index, std::move(wordsData));
    };

    for(size_t i = 0; i < options->constructed; i++)
        add(generator.constructed(), true);
    for(size_t i = 0; i < options->random; i++)
        add(generator.random(), false);

    if(disagreements != 0 || unsolvedConstructed != 0) {
        std::println(stderr, "lbs and gpt disagree on {} puzzles, lbs missed {} constructed puzzles", disagreements, unsolvedConstructed);
        return 1;
    }

    std::ranges::stable_sort(puzzles, {}, &puzzle::group);
    register_benchmarks(puzzles, list);

    std::println(
        "puzzles: {} constructed, {} random (seed {}), word list: {}",
        options->constructed,
        options->random,
        options->seed,
        WORD_LIST_PATH
    );

    percentile_reporter reporter{};
    benchmark::RunSpecifiedBenchmarks(&reporter);
    benchmark::Shutdown();
}
