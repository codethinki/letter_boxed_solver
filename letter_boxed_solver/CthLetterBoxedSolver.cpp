#include "CthLetterBoxedSolver.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

namespace cth {


vector<string> loadTextFile(const string_view path) {
    vector<string> data{};
    fstream file(path.data());
    string line;
    while(getline(file, line)) data.push_back(line);

    file.close();
    return data;
}
template<typename RandIt>
void writeTextFile(const string_view filepath, RandIt data_begin, const RandIt& data_end, const char delimiter) {
    ofstream file(filepath.data(), ios::ate);

    while(data_begin != data_end) {
        file << *data_begin << delimiter;
        ++data_begin;
    }
    file.close();
}

size_t countWords(const string_view filepath, const char list_delimiter) {
    ifstream file(filepath.data());
    return count(istreambuf_iterator<char>(file), istreambuf_iterator<char>(), list_delimiter);
}

void prepareWordList(const string_view filepath, const char list_delimiter) {
    if(countWords(filepath, list_delimiter) < 20) {
        cout << '\n' << "ERROR: LIST INVALID\n TIP: undef PREPARE_WORD_LIST or change the list delimiter" << endl;
        exit(EXIT_FAILURE);
    }

    vector<string> content = loadTextFile(filepath);

    vector<uint32_t> removable{};
#pragma omp parallel for schedule(dynamic) shared(content, removable)
    for(int i = 0; i < content.size(); i++) {
        string& line = content[i];
        bool valid = line.size() >= 3;

        for(int k = 0; valid && k < line.size(); k++) {
            char& c = line[k];
            if(c >= 'a' && c <= 'z') continue;
            if(c >= 'A' && c <= 'Z') {
                c += 'a' - 'A';
                continue;
            }
            valid = false;
        }
#pragma omp critical
        if(!valid) removable.push_back(i);
    }
    ranges::sort(removable);
    for(int i = 0; i < removable.size(); i++) content.erase(content.begin() + removable[i] - i);

    ranges::sort(content);

    writeTextFile(filepath, content.begin(), content.end(), ',');
}

vector<string> loadDictionaryFromWordlist(const string_view dictionary_path, const string_view valid_chars) {
    vector<string> words{};
    ifstream file(dictionary_path.data());

    if(!file.is_open()) {
        cout << "\nERROR: no dictionary found\n";
        exit(EXIT_FAILURE);
    }

    bool finished = false;
#pragma omp parallel shared(finished)
    {
        constexpr uint32_t linesPerRead = 1000;
        vector<string> validWords{};
        vector<string> lines{linesPerRead};
        while(!finished) {
#pragma omp critical
            {
                for(int i = 0; i < linesPerRead && !finished; i++) finished = !static_cast<bool>(getline(file, lines[i], ','));
            }
            for(auto& line : lines) {
                size_t prevSide = valid_chars.find(line[0]);
                for(int i = 1; prevSide != string::npos && i < line.size(); i++) {
                    const size_t side = valid_chars.find(line[i]);

                    prevSide = (prevSide / CHARS_PER_SIDE == side / CHARS_PER_SIDE) ? string::npos : side;
                }
                if(prevSide == string::npos) continue;

                validWords.push_back(line);
            }
        }

#pragma omp critical
        words.insert(words.end(), validWords.begin(), validWords.end());
    }
    ranges::sort(words);

    file.close();

    words.push_back(" "); // add a space to the end of the list to avoid out of bounds errors
    return words;
}

vector<string> calcFixedSizeSolutions(const int word_count, const string_view sorted_valid_chars, const array<uint32_t, 25>& dictionary_entries,
    const vector<string>& dictionary) {
    vector<string> solutions{};

    const int dictionarySize = dictionary.size() - 1; //dictionary .size() - 1 because of the appended empty word at the end

#pragma omp parallel for shared(dictionary_entries, dictionary, sorted_valid_chars, dictionarySize, solutions)
    for(uint32_t i = 0; i < dictionarySize; i++) {
        vector<uint32_t> offsets(word_count);
        vector<uint32_t> offsetsMax(word_count);
        //set the first offset based on the current fixed word

        auto offsetMax = [dictionary_entries, dictionary](const size_t prev_offset) {
            return dictionary_entries[dictionary[prev_offset].back() - 'a' + 1];
        };
        auto offset = [dictionary_entries, dictionary](const size_t prev_offset) { return dictionary_entries[dictionary[prev_offset].back() - 'a']; };

        offsets[0] = i;
        offsetsMax[0] = dictionarySize;

        for(int k = 1; k < offsetsMax.size(); k++) offsetsMax[k] = offsetMax(offsets[k - 1]);

        int update = 1;
        while(i == offsets[0]) {
            for(; update < offsets.size(); update++) {
                offsetsMax[update] = offsetMax(offsets[update - 1]);
                offsets[update] = offset(offsets[update - 1]);
            }

            for(; offsets.back() < offsetsMax.back(); ++offsets.back()) {
                string solution{};
                for(int k = 0; k < offsets.size(); k++) solution += dictionary[offsets[k]];
                bool valid = true;
                for(int k = 0; valid && k < sorted_valid_chars.size(); k++) valid = solution.contains(sorted_valid_chars[k]);

                if(valid) {
                    solution = "";
                    for(int k = 0; k < offsets.size(); k++) {
                        solution += dictionary[offsets[k]];
                        solution += ", ";
                    }
#pragma omp critical
                    solutions.push_back(solution.substr(0, solution.size() - 2));
                }
            }

            update = offsets.size() - 2;
            while(update >= 0 && (++offsets[update]) >= offsetsMax[update]) --update;
            ++update;
        }
    }
    return solutions;
}

vector<string> calcBestSolutions(const string_view sorted_valid_chars, const vector<string>& dictionary) {
    vector<string> solutions{};
    const array<uint32_t, 25> dictionaryEntries = [sorted_valid_chars, dictionary]() {
        array<uint32_t, 25> arr{};
        for(int i = 0; i < sorted_valid_chars.size(); i++) {
            int k = 0;
            for(; k < dictionary.size() - 1 && dictionary[k][0] != sorted_valid_chars[i]; k++);

            arr[sorted_valid_chars[i] - 'a'] = k;
            if(i > 0) arr[sorted_valid_chars[i - 1] - 'a' + 1] = k;
        }
        arr[sorted_valid_chars.back() - 'a' + 1] = dictionary.size() - 1;

        return arr;
    }();


    for(int i = MIN_WORDS; solutions.empty() && i <= MAX_WORDS; i++)
        solutions = calcFixedSizeSolutions(i, sorted_valid_chars, dictionaryEntries,
            dictionary);

    ranges::sort(solutions, [](const string_view a, const string_view b) { return a.size() < b.size(); });

    if(solutions.size() > MAX_SOLUTIONS) solutions.resize(MAX_SOLUTIONS);

    return solutions;
}
string getLetterBoxedSides() {
    string validChars{};

    cout << "input the characters of one rectangle each (lowercase):\n";
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
