#pragma once
#include <fstream>
#include <iostream>
#include <string>

#include "CthGlobalVariables.hpp"

#include <cth/io/log.hpp>


namespace cth {
using namespace std;
inline vector<string> loadTextFile(std::filesystem::path path, char const delimiter) {
    vector<string> data{};
    fstream file(path);
    string line;
    while(getline(file, line, delimiter)) data.push_back(line);

    file.close();
    return data;
}

inline vector<char> loadWordList(std::filesystem::path const& path) {
    CTH_WARN(path.extension() != OPTIMIZED_EXTENSION, "probably not an optimized wordlist and invalid") {}

    std::ifstream in{path, std::ios::binary | std::ios::ate};
    size_t const size = in.tellg();    
    std::vector<char> data(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(data.data(), data.size());
    return data;
}


template<typename RandIt>
void writeTextFile(string_view const filepath, RandIt data_begin, RandIt const& data_end, char const delimiter) {
    ofstream file(filepath.data(), ios::ate);

    while(data_begin != data_end) {
        file << *data_begin << delimiter;
        ++data_begin;
    }
    file.close();
}

inline size_t countWords(string_view const filepath, char const list_delimiter) {
    ifstream file(filepath.data());
    return count(istreambuf_iterator<char>(file), istreambuf_iterator<char>(), list_delimiter);
}

inline string getLetterBoxedSides() {
    string validChars{};

    std::println("input the characters of one rectangle side each (lowercase):\n");
    for(int i = 0; i < SIDES; i++) {
        std::println("side [{}]: ", to_string(i + 1));
        bool valid = true;
        string input;
        do {
            if(!valid) std::println("  invalid, try again: ");
            cin >> input;
            valid = input.size() == CHARS_PER_SIDE;
            for(size_t k = 0; k < CHARS_PER_SIDE && valid; k++) valid = input[k] >= 'a' && input[k] <= 'z' && !validChars.contains(input[k]);
        } while(!valid);
        validChars += input;
    }

    return validChars;
}

}
