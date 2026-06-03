#pragma once

namespace lbs {
constexpr char FIRST = 'a';
constexpr char LAST = 'z';

constexpr size_t CHARACTERS = LAST - FIRST + 1;

constexpr size_t SIDES = 4;
constexpr size_t CHARS_PER_SIDE = 3;

constexpr auto BOX_CHARS = SIDES * CHARS_PER_SIDE;

constexpr size_t MAX_SEARCH_SIZE = 3;
constexpr size_t MIN_SOLUTION_SIZE = 1;
constexpr size_t MAX_SOLUTIONS = 20;

static constexpr size_t STATE_CACHE_MIN_DEPTH = 2;

}
