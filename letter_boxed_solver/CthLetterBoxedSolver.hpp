#pragma once

#include <algorithm>
#include <array>
#include<thread>
#include <vector>

constexpr uint32_t MIN_WORDS = 1;
constexpr uint32_t MAX_WORDS = 3;
constexpr uint32_t CHARS_PER_SIDE = 3;
constexpr uint32_t SIDES = 4;
constexpr uint32_t CHARS = SIDES * CHARS_PER_SIDE;

constexpr uint32_t MAX_SOLUTIONS = 800;


namespace cth {

using namespace std;

vector<string> loadTextFile(string_view path);

template<typename RandIt>
void writeTextFile(string_view filepath, RandIt data_begin, const RandIt& data_end, char delimiter = '\n');

size_t countWords(string_view filepath, char list_delimiter = '\n');

void prepareWordList(string_view filepath, char list_delimiter = '\n');


vector<string> loadDictionaryFromWordlist(string_view dictionary_path, string_view valid_chars);

vector<string> calcFixedSizeSolutions(int word_count, string_view sorted_valid_chars,
    const array<uint32_t, 25>& dictionary_entries, const vector<string>& dictionary);

vector<string> calcBestSolutions(string_view sorted_valid_chars, const vector<string>& dictionary);



string getLetterBoxedSides();

}
