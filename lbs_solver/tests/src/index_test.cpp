#include "solver/index.hpp"

#include <gtest/gtest.h>
#include <print>
#include <string_view>


namespace lbs {
TEST(Index, ctor) {
    constexpr std::string_view wordList{
        "a,b,c,",
    };
    constexpr size_t expectedWordC = 3;

    auto const splits = split(wordList, ',', 1);

    ASSERT_EQ(splits[0], 1);
    ASSERT_EQ(splits[1], 3);
    ASSERT_EQ(splits[2], 5);

    auto const compressionIndex = gen_compression_index("axxbyyczzzzz");

    auto const& [actualWords, actualWordHashes] = filter_and_hash(
        compressionIndex,
        wordList,
        splits
    );

    ASSERT_EQ(actualWords.size(), expectedWordC);

    ASSERT_EQ(actualWords[0], "a");
    ASSERT_EQ(actualWords[1], "b");
    ASSERT_EQ(actualWords[2], "c");

    ASSERT_EQ(actualWordHashes[0], bit_flag('a', compressionIndex));
    ASSERT_EQ(actualWordHashes[1], bit_flag('b', compressionIndex));
    ASSERT_EQ(actualWordHashes[2], bit_flag('c', compressionIndex));


}
}
