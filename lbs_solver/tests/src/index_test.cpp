#include "solver/index.hpp"

#include <gtest/gtest.h>
#include <print>
#include <string>
#include <string_view>
#include <vector>


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

namespace {
    std::vector<size_t> scalar_split(std::string_view data, char separator) {
        std::vector<size_t> separatorIdxs{};
        for(size_t i = 0; i < data.size(); i++)
            if(data[i] == separator)
                separatorIdxs.push_back(i);
        return separatorIdxs;
    }
}

static_assert(split(std::string_view{"ab,cd,ef"}, ',') == std::vector<size_t>{2, 5});

TEST(Index, split_every_size) {
    std::string data{};
    for(size_t i = 0; data.size() < 300; i++)
        data += std::string(i % 13, 'a') + ',';

    for(size_t size = 0; size <= data.size(); size++) {
        std::string_view const prefix{data.data(), size};
        EXPECT_EQ(split(prefix, ','), scalar_split(prefix, ',')) << "size " << size;
    }
}

TEST(Index, split_only_separators) {
    std::string const data(300, ',');
    EXPECT_EQ(split(data, ','), scalar_split(data, ','));
}
}
