#pragma once

#include <algorithm>
#include <array>
#include <assert.h>
#include<fstream>
#include <iostream>
#include <iostream>
#include <omp.h>
#include<thread>
#include <vector>

inline constexpr uint32_t MIN_WORDS = 2;
inline constexpr uint32_t MAX_WORDS = 2;
inline constexpr uint32_t CHARS_PER_SIDE = 3;
inline constexpr uint32_t SIDES = 4;
inline constexpr uint32_t CHARS = SIDES * CHARS_PER_SIDE;


namespace cth {

    using namespace std;

    vector<string> loadTextFile(string_view path);

    template<typename RandIt>
    void writeTextFile(string_view filepath, RandIt data_begin, const RandIt& data_end, char delimiter = '\n');

    size_t countWords(string_view filepath, char list_delimiter = '\n');

    void prepareWordList(string_view filepath, char list_delimiter = '\n');



    vector<string> loadDictionaryFromWordlistv2(string_view dictionary_path, string_view valid_chars);

    vector<string> loadDictionaryFromWordlist(string_view dictionary_path, string_view valid_chars);

    vector<string> calcFixedSizeSolutions(int word_count, string_view sorted_valid_chars,
        const array<uint32_t, 25>& dictionary_entries, const vector<string>& dictionary);

    vector<string> calcBestSolutions(string_view sorted_valid_chars, const vector<string>& dictionary);



    string getLetterBoxedSides();

}