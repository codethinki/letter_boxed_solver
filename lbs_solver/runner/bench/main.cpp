#include "gpt_runner.hpp"
#include "percentile_reporter.hpp"

#include "solver/solver.hpp"

#include <benchmark/benchmark.h>
#include <cth/io/file.hpp>

#include <algorithm>
#include <cstddef>
#include <format>
#include <fstream>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace lbs::bench {

constexpr std::string_view WORD_LIST_PATH = "assets/words_easy.txt";
constexpr std::string_view PUZZLES_PATH = "assets/nyt_puzzles.txt";

struct word_list {
    std::vector<std::byte> data;
    std::string_view csv;
    std::vector<size_t> splits;
};

struct puzzle {
    std::string date;
    std::string letters;
    std::string group;
    compression_index_t compressionIndex;
    words_data wordsData;
};

[[nodiscard]] word_list load_word_list() {
    auto data = cth::io::file::read<std::byte>(WORD_LIST_PATH);
    std::string_view const csv{reinterpret_cast<char const*>(data.data()), data.size()};
    auto splits = split(csv, ',');
    return {std::move(data), csv, std::move(splits)};
}

/**
 * reads the puzzles, one `<date> <letters>` per line
 */
[[nodiscard]] std::vector<std::pair<std::string, std::string>> load_puzzles() {
    std::ifstream file{std::string{PUZZLES_PATH}};

    std::vector<std::pair<std::string, std::string>> puzzles{};
    std::string date, letters;
    while(file >> date >> letters)
        puzzles.emplace_back(std::move(date), std::move(letters));
    return puzzles;
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
        auto const name = [&p](std::string_view family) { return std::format("{}/{}/{}", family, p.group, p.date); };

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
    if(benchmark::ReportUnrecognizedArguments(argCount, args.data()))
        return 1;
    benchmark::SetDefaultTimeUnit(benchmark::kMicrosecond);

    auto const list = load_word_list();

    std::vector<puzzle> puzzles{};
    LetterBoxedSolver gpt{};
    size_t disagreements = 0;

    for(auto& [date, letters] : load_puzzles()) {
        auto const index = lbs::gen_compression_index(letters);
        auto wordsData = lbs::filter_and_hash(index, list.csv, list.splits);

        bool const solvable = !lbs::solve(index, wordsData.words, wordsData.wordHashes).empty();
        disagreements += solvable == gpt.run(letters, WORD_LIST_PATH).empty();

        std::string group = solvable ? "solvable" : "unsolvable";
        puzzles.emplace_back(std::move(date), std::move(letters), std::move(group), index, std::move(wordsData));
    }

    if(puzzles.empty()) {
        std::println(stderr, "no puzzles in {}", PUZZLES_PATH);
        return 1;
    }
    if(disagreements != 0) {
        std::println(stderr, "lbs and gpt disagree on {} puzzles", disagreements);
        return 1;
    }

    std::ranges::stable_sort(puzzles, {}, &puzzle::group);
    register_benchmarks(puzzles, list);

    std::println(
        "puzzles: {} nyt ({} to {}), word list: {}",
        puzzles.size(),
        std::ranges::min(puzzles, {}, &puzzle::date).date,
        std::ranges::max(puzzles, {}, &puzzle::date).date,
        WORD_LIST_PATH
    );

    percentile_reporter reporter{};
    benchmark::RunSpecifiedBenchmarks(&reporter);
    benchmark::Shutdown();
}
