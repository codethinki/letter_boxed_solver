#pragma once
#include "settings.hpp"

#include <array>
#include <cstdint>
#include <limits>
#include <span>


namespace lbs {
[[nodiscard]] constexpr size_t idx(char c) {
    return c - FIRST;
}

using char_mask_t = uint32_t;

constexpr char_mask_t solution_mask = []() {
    char_mask_t out{};

    for(size_t i = 0; i < BOX_CHARS; i++)
        out |= char_mask_t{1} << i;
    return out;
}();

constexpr char_mask_t INVALID_CHAR_FLAG = [] {
    char_mask_t m{};
    for(size_t i = 0; i < BOX_CHARS; i++)
        m |= char_mask_t{1} << i;
    return m;
}();


using compression_index_t = std::array<char_mask_t, CHARACTERS>;
using box_chars_view_t = std::span<char const, BOX_CHARS>;

[[nodiscard]] constexpr char_mask_t bit_flag(char c, compression_index_t const& flags) {
    return char_mask_t{1} << flags[idx(c)];
}


[[nodiscard]] constexpr char_mask_t mask_of(box_chars_view_t chars, compression_index_t const& flags) {
    char_mask_t out = 0;
    for(auto& c : chars)
        out |= bit_flag(c, flags);
    return out;
}



}
