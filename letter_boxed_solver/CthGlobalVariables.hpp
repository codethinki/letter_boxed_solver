#pragma once
#include <array>
#include <string>


namespace cth {


constexpr std::string_view OPTIMIZED_EXTENSION = ".cth";

constexpr std::array<char const*, 2> WORDLIST_PATHS = { {"resources/words_easy.txt", "resources/words_hard.txt"} };

constexpr size_t MIN_WORDS = 1;
constexpr size_t MAX_WORDS = 3;
constexpr size_t MIN_SOLUTIONS = 2;
constexpr size_t MAX_SOLUTIONS = 80;

constexpr char LIST_DELIMITER = ','; //delimiter of your unprepared wordlist
constexpr size_t ROW_SIZE = 32;
constexpr size_t CHAR_COUNT_LENGTH = 1;
constexpr size_t MAX_WORD_SIZE = ROW_SIZE - CHAR_COUNT_LENGTH;





constexpr size_t CHARS_PER_SIDE = 3;
constexpr size_t SIDES = 4;
constexpr size_t CHARS = SIDES * CHARS_PER_SIDE;
}
