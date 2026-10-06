#include "solver/solver.hpp"

#include <gtest/gtest.h>

#include <cth/io/file.hpp>
#include <print>


namespace lbs {

TEST(solver, real_world) {
    auto const data = cth::io::file::read<std::byte>("assets/words_easy.txt");
    std::string_view csv{reinterpret_cast<char const*>(data.data()), data.size()};
    auto splits = split(csv, ',');

    constexpr auto compressionIndex = gen_compression_index("ouserftlbayh");
    auto const& [words, wordHashes] = filter_and_hash(compressionIndex, csv, splits);

    auto const solutions = solve(compressionIndex, words, wordHashes);
    EXPECT_FALSE(solutions.empty());

    std::println("{}", solutions);
}

TEST(solver, letters_only_inside_words) {
    auto const data = cth::io::file::read<std::byte>("assets/words_easy.txt");
    std::string_view const csv{reinterpret_cast<char const*>(data.data()), data.size()};
    auto const splits = split(csv, ',');

    auto const compressionIndex = gen_compression_index("ancgwuytsvoq");
    auto const& [words, wordHashes] = filter_and_hash(compressionIndex, csv, splits);

    EXPECT_FALSE(solve(compressionIndex, words, wordHashes).empty());
}

} // namespace lbs
