#pragma once

#include <algorithm>
#include <array>
#include<thread>
#include <vector>

#include "CthUtils.hpp"


namespace cth {

using namespace std;

void prepareWordList(string_view filepath, char list_delimiter = '\n');

vector<string> loadDictionaryFromWordlist(string_view wordlist_path, string_view valid_chars);

vector<string> calcFixedSizeSolutions(int word_count, string_view sorted_valid_chars,
    const array<uint32_t, 25>& dictionary_entries, const vector<string>& dictionary);

vector<string> calcBestSolutions(string_view sorted_valid_chars, const vector<string>& dictionary);
}
