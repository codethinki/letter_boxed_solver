#pragma once
#include <array>
#include <string>


namespace cth {
using namespace std;

constexpr array<string, 2> WORDLIST_PATHS = { {"words_easy.txt", "words_hard.txt"} };

constexpr uint32_t MIN_WORDS = 1;
constexpr uint32_t MAX_WORDS = 3;
constexpr uint32_t MIN_SOLUTIONS = 2;
constexpr uint32_t MAX_SOLUTIONS = 80;

//#define PREPARE_WORD_LIST //uncomment this line to enable wordlist preparation PREPARE A WORDLIST ONCE
#ifdef PREPARE_WORD_LIST
constexpr char LIST_DELIMITER = '\n'; //delimiter of your unprepared wordlist
#endif


constexpr uint32_t CHARS_PER_SIDE = 3;
constexpr uint32_t SIDES = 4;




constexpr uint32_t CHARS = SIDES * CHARS_PER_SIDE;
}
