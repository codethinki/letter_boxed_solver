#pragma once
#include "solver/util.hpp"

#include <cth/io/log.hpp>

#include <cstdint>
#include <filesystem>
#include <ranges>
#include <span>
#include <string_view>
#include <vector>


namespace lbs {
[[nodiscard]] constexpr std::vector<size_t> split(
    std::span<char const> data,
    char separator,
    size_t separator_dist_guess = 3
) {
    using mask_t = uint64_t;
    std::vector<size_t> indices{};
    indices.reserve(data.size() / separator_dist_guess);

    static constexpr size_t BLOCK_SIZE = sizeof(mask_t) * 8;

    auto const chunkedEnd = (data.size() / BLOCK_SIZE) * BLOCK_SIZE;

    static constexpr auto extract_indices = [](
        std::vector<size_t>& buffer,
        mask_t mask,
        size_t offset
    ) {
        while(mask != 0) {
            auto const idx = std::countr_zero(mask);
            buffer.push_back(offset + idx);
            mask &= mask - mask_t{1}; // Clear lowest set bit
        }
    };

    // simd friendly processing
    for(size_t blockBegin = 0; blockBegin < chunkedEnd; blockBegin += BLOCK_SIZE) {
        mask_t blockMask = 0;

        // simd separator into mask
        for(size_t i = 0; i < BLOCK_SIZE; i++)
            if(data[blockBegin + i] == separator)
                blockMask |= mask_t{1} << i;

        extract_indices(indices, blockMask, blockBegin);
    }

    //tail cleanup
    for(size_t i = chunkedEnd; i < data.size(); i++)
        if(data[i] == separator)
            indices.push_back(i);

    return indices;
}

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
 * @param compression_index to base masks on
 * @return uint32_t masks
 */
[[nodiscard]] constexpr auto gen_char_filter_masks(
    compression_index_t const& compression_index
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


    for(size_t i = 0; i < compression_index.size(); i++) {
        auto const& compressedIdx = compression_index[i];
        char_mask_t flag{};
        if(compressedIdx == INVALID_CHAR_FLAG)
            flag = INVALID_CHAR_FLAG;
        else {
            auto const side = compression_index[i] / CHARS_PER_SIDE;
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
