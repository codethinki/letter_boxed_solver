#include "solver/solver.hpp"

#include <gtest/gtest.h>

#include <cth/win/io.hpp>
#include <print>

#include <chrono>


namespace lbs {

using clock = std::chrono::high_resolution_clock;

TEST(solver, real_world) {

    using fms = std::chrono::duration<double, std::milli>;

    auto const startLoad = clock::now();
    auto const data = cth::win::io::read_unbuffered("assets/words_easy.txt");
    auto const endLoad = clock::now();

    auto const startSplit = clock::now();
    std::string_view csv{reinterpret_cast<char const*>(data.data()), data.size()};
    auto splits = split(csv, ',');
    auto const endSplit = clock::now();

    auto const startFiltering = clock::now();
    constexpr auto compressionIndex = gen_compression_index("ouserftlbayh");
    auto const& [words, wordHashes] = filter_and_hash(compressionIndex, csv, splits);
    auto const endFiltering = clock::now();

    auto const startSolve = clock::now();
    auto const solutions = solve(compressionIndex, words, wordHashes);
    auto const endSolve = clock::now();

    std::println("{}", words);
    std::println("{}", solutions);

    std::println("loaded data in {}", fms{endLoad - startLoad});
    std::println("split words in {}", fms{endSplit - startSplit});
    std::println("indexed in {}", fms{endFiltering - startFiltering});
    std::println("solved in {}", fms{endSolve - startSolve});
}

} // namespace lbs
