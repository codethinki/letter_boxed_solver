#pragma once
#include "solver/util.hpp"

#include <cth/io/log.hpp>

#include <array>
#include <cstdint>
#include <filesystem>
#include <ranges>
#include <span>
#include <string_view>
#include <vector>

#if __has_include(<simd>)
#include <simd>
#endif


namespace lbs {
[[nodiscard]] constexpr std::vector<size_t> split(
    std::span<char const> data,
    char separator,
    size_t separatorDistGuess = 3
);

#if defined(__cpp_lib_simd) || defined(__glibcxx_simd)
[[nodiscard]] constexpr std::vector<size_t> split(
    std::span<char const> data,
    char separator,
    size_t separatorDistGuess
) {
    static constexpr size_t BLOCK_SIZE = sizeof(uint64_t) * 8;
    using block_t = std::simd::vec<char, BLOCK_SIZE>;


    std::vector<size_t> separatorIdxs{};
    separatorIdxs.reserve(data.size() / separatorDistGuess);

    auto const blocks = data.size() / BLOCK_SIZE;

    auto const uncheckedEnd = BLOCK_SIZE * blocks;

    auto extract_separator_idxs = [&separatorIdxs](size_t offset, uint64_t mask) {
        while(mask != 0) {
            separatorIdxs.push_back(offset + std::countr_zero(mask));
            mask &= mask - 1;
        }
    };


    for(size_t i = 0; i < uncheckedEnd; i += block_t::size) {
        auto simdBlock = std::simd::unchecked_load<block_t>(data.subspan(i, block_t::size));
        extract_separator_idxs(i, (simdBlock == block_t{separator}).to_ullong());
    }

    std::array<char, BLOCK_SIZE> lastChars{};
    std::ranges::copy(data.subspan(uncheckedEnd), lastChars.begin());
    auto lastBlock = std::simd::unchecked_load<block_t>(lastChars);

    extract_separator_idxs(
        uncheckedEnd,
        (lastBlock == block_t{separator}).to_ullong()
    );

    return separatorIdxs;
}



#else

[[nodiscard]] constexpr std::vector<size_t> split(
    std::span<char const> data,
    char separator,
    size_t separatorDistGuess
) {
    static constexpr size_t BLOCK_SIZE = 256;

    std::vector<size_t> separatorIdxs{};
    separatorIdxs.reserve(data.size() / separatorDistGuess);

    std::array<size_t, BLOCK_SIZE> blockSeparatorIdxs{};

    for(size_t i = 0; i < data.size(); i += BLOCK_SIZE) {
        auto const blockEnd = std::min(i + BLOCK_SIZE, data.size());
        size_t separatorC = 0;

        for(size_t j = i; j < blockEnd; j++) {
            blockSeparatorIdxs[separatorC] = j;
            separatorC += data[j] == separator;
        }

        separatorIdxs.append_range(std::span{blockSeparatorIdxs}.first(separatorC));
    }

    return separatorIdxs;
}
#endif

}

namespace lbs {

/**
 * compresses the alphabet into a bitmask with only allowed chars. invalid chars have all bits set.
 * @param characters allowed in instance
 * @return array with bitshift indices.
 */
[[nodiscard]] constexpr auto gen_compression_index(box_chars_view_t characters) {
    static_assert(BOX_CHARS < sizeof(char_mask_t) * 8, "can't fit compression");

    std::array<char_mask_t, CHARACTERS> compressionIndex{};
    compressionIndex.fill(INVALID_CHAR_FLAG);

    for(char_mask_t i = 0; i < characters.size(); i++)
        compressionIndex[idx(characters[i])] = i;

    return compressionIndex;
}

[[nodiscard]] constexpr auto gen_compression_index(std::string_view characters) {
    CTH_CRITICAL(characters.size() != BOX_CHARS, "character count not allowed") {}
    return gen_compression_index(box_chars_view_t{characters.data(), BOX_CHARS});
}

/**
 * Gens the masks used for filtering. 
 * @details Masks set whole sides to filter same side char sequences. 
 *  Invalid chars overlap with everything. Any overlap => invalid word
 *
 * @param compressionIndex to base masks on
 * @return uint32_t masks
 */
[[nodiscard]] constexpr auto gen_char_filter_masks(
    compression_index_t const& compressionIndex
) {
    // a side mask sets all bits of a side to 1
    static constexpr auto SIDE_MASKS = [] {
        // first side (first n bits to 1)
        static char_mask_t constexpr DEFAULT_SIDE_MASK = (char_mask_t{1} << CHARS_PER_SIDE) - 1;

        std::array<char_mask_t, SIDES> sideMasks{};
        for(size_t i = 0; i < sideMasks.size(); i++)
            // shift first side left for each other one
            sideMasks[i] = DEFAULT_SIDE_MASK << (CHARS_PER_SIDE * i);
        return sideMasks;
    }();

    std::array<char_mask_t, CHARACTERS> filterFlags{};
    filterFlags.fill(INVALID_CHAR_FLAG);


    for(size_t i = 0; i < compressionIndex.size(); i++) {
        auto const& compressedIdx = compressionIndex[i];
        char_mask_t flag{};
        if(compressedIdx == INVALID_CHAR_FLAG)
            flag = INVALID_CHAR_FLAG;
        else {
            auto const side = compressionIndex[i] / CHARS_PER_SIDE;
            flag = SIDE_MASKS[side];
        }
        filterFlags[i] = flag;
    }


    return filterFlags;
}

struct words_data {
    std::vector<std::string_view> words;
    std::vector<char_mask_t> wordHashes;
};

[[nodiscard]] constexpr words_data filter_and_hash(
    compression_index_t const& compression_index,
    std::string_view csv,
    std::span<size_t const> splits
) {
    auto const charFilterMasks = gen_char_filter_masks(compression_index);

    static constexpr size_t FILTER_FREQ = 2;

    words_data out{};
    out.words.reserve(splits.size() / FILTER_FREQ);
    out.wordHashes.reserve(splits.size() / FILTER_FREQ);

    size_t begin = 0;

    // <= to handle tail
    for(size_t splitI = 0; splitI <= splits.size(); splitI++) {
        auto const end = splitI == splits.size() ? csv.size() : splits[splitI];
        auto const size = end - begin;
        if(size > 0) {
            auto const word = csv.substr(begin, size);

            char_mask_t wordHash = bit_flag(word[0], compression_index);
            size_t i = 1;

            for(; i < size; i++) {
                auto const& charMask = charFilterMasks[idx(word[i])];
                auto const& prevCharMask = charFilterMasks[idx(word[i - 1])];
                wordHash |= bit_flag(word[i], compression_index);
                if((charMask & prevCharMask) != 0)
                    break;
            }

            if(i == size) {
                out.words.push_back(word);
                out.wordHashes.push_back(wordHash);
            }
        }
        begin = end + 1;
    }

    return out;
}

}
