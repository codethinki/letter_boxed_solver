#pragma once
#include <fstream>
#include <iostream>
#include <string>

#include "CthGlobalVariables.hpp"


namespace cth {
using namespace std;
    inline vector<string> loadTextFile(const string_view path, const char delimiter) {
        vector<string> data{};
        fstream file(path.data());
        string line;
        while (getline(file, line, delimiter)) data.push_back(line);

        file.close();
        return data;
    }
    template<typename RandIt>
    void writeTextFile(const string_view filepath, RandIt data_begin, const RandIt& data_end, const char delimiter) {
        ofstream file(filepath.data(), ios::ate);

        while (data_begin != data_end) {
            file << *data_begin << delimiter;
            ++data_begin;
        }
        file.close();
    }

    inline size_t countWords(const string_view filepath, const char list_delimiter) {
        ifstream file(filepath.data());
        return count(istreambuf_iterator<char>(file), istreambuf_iterator<char>(), list_delimiter);
    }

    inline string getLetterBoxedSides() {
        string validChars{};

        cout << "input the characters of one rectangle side each (lowercase):\n";
        for (int i = 0; i < SIDES; i++) {
            cout << "side [" << to_string(i + 1) << "]: ";
            bool valid = true;
            string input;
            do {
                if (!valid) cout << "  invalid, try again: ";
                cin >> input;
                valid = input.size() == CHARS_PER_SIDE;
                for (int k = 0; k < CHARS_PER_SIDE && valid; k++) valid = input[k] >= 'a' && input[k] <= 'z' && !validChars.contains(input[k]);
            } while (!valid);
            validChars += input;
        }

        return validChars;
    }

}
