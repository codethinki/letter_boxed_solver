#include <iostream>
#include <string>

#include "CthLetterBoxedSolver.hpp"
#include "CthUtils.hpp"


namespace cth {

#ifdef PREPARE_WORD_LIST
void prepareWordList(const string_view filepath) {
    if(countWords(filepath, LIST_DELIMITER) < 20) {
        cout << '\n' << "ERROR: LIST INVALID\n TIP: undef PREPARE_WORD_LIST or change the list delimiter" << endl;
        exit(EXIT_FAILURE);
    }
    else if (LIST_DELIMITER == ',') {
        cout << '\n' << "WARNING: preparing list with the programs default delimiter!" << "\n make sure that list isn't already prepared";
    }

    vector<string> content = loadTextFile(filepath, LIST_DELIMITER);

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
        if(!valid) {
#pragma omp critical
            removable.push_back(i);
        }
    }
    ranges::sort(removable);
    for(int i = 0; i < removable.size(); i++) content.erase(content.begin() + removable[i] - i);

    ranges::sort(content);

    writeTextFile(filepath, content.begin(), content.end(), ',');
}
#endif


vector<string> loadDictionaryFromWordlist(const string_view wordlist_path, const string_view valid_chars) {
    vector<string> words{};
    ifstream file(wordlist_path.data());

    if(!file.is_open()) {
        cout << "\nERROR: no dictionary found\n searched for: " << wordlist_path.data() << endl;
        exit(EXIT_FAILURE);
    }

    bool finished = false;
#pragma omp parallel shared(finished)
    {
        vector<string> validWords{};
        while(!finished) {
            string line;
#pragma omp critical
            finished = !static_cast<bool>(getline(file, line, ','));

            size_t prevSide = valid_chars.find(line[0]);
            for(int i = 1; prevSide != string::npos && i < line.size(); i++) {
                const size_t side = valid_chars.find(line[i]);

                prevSide = (prevSide / CHARS_PER_SIDE == side / CHARS_PER_SIDE) ? string::npos : side;
            }
            if(prevSide != string::npos) validWords.push_back(line);
        }

#pragma omp critical
        words.insert(words.end(), validWords.begin(), validWords.end());
    }
    ranges::sort(words);

    file.close();

    words.push_back(" "); // add a space to the end of the list to avoid out of bounds errors
    return words;
}

array<uint32_t, 27> calcDictionaryEntries(const string_view sorted_valid_chars, const vector<string>& dictionary) {
    array<uint32_t, 27> arr{};
    for(int i = 0; i < sorted_valid_chars.size(); i++) {
        int k = 0;
        for(; k < dictionary.size() - 1 && dictionary[k][0] != sorted_valid_chars[i]; k++);

        arr[sorted_valid_chars[i] - 'a'] = k;
        if(i > 0) arr[sorted_valid_chars[i - 1] - 'a' + 1] = k;
    }
    arr[sorted_valid_chars.back() - 'a' + 1] = dictionary.size() - 1;

    return arr;
}


vector<string> calcFixedSizeSolutions(const int word_count, const string_view sorted_valid_chars, const array<uint32_t, 27>& dictionary_entries,
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

                if(!valid) continue;


                solution = "";
                for(int k = 0; k < offsets.size(); k++) {
                    solution += dictionary[offsets[k]];
                    solution += ", ";
                }
#pragma omp critical
                solutions.push_back(solution.substr(0, solution.size() - 2));
            }

            update = offsets.size() - 2;
            while(update >= 0 && (++offsets[update]) >= offsetsMax[update]) --update;
            ++update;
        }
    }
    return solutions;
}

vector<string> solve(const string_view letter_boxed_sides) {
    static auto isSmaller = [](const string_view a, const string_view b) { return a.size() < b.size(); };


    string sortedSideChars{letter_boxed_sides.data()};
    ranges::sort(sortedSideChars);

    vector<string> solutions{};

    array<vector<string>, WORDLIST_PATHS.size()> dictionaries{};
    array<array<uint32_t, 27>, WORDLIST_PATHS.size()> dictionaryEntries{};



    for(int words = MIN_WORDS; words <= MAX_WORDS && solutions.size() < MIN_SOLUTIONS; words++) {
        for(int listId = 0; listId < WORDLIST_PATHS.size() && solutions.size() < MIN_SOLUTIONS; listId++) {
            if(words == MIN_WORDS) {
                dictionaries[listId] = loadDictionaryFromWordlist(WORDLIST_PATHS[listId], letter_boxed_sides);
                dictionaryEntries[listId] = calcDictionaryEntries(sortedSideChars, dictionaries[listId]);
            }
            vector<string> rawSolutions = calcFixedSizeSolutions(words, sortedSideChars, dictionaryEntries[listId], dictionaries[listId]);
            ranges::sort(rawSolutions, isSmaller);

            if(const uint32_t maxSize = MAX_SOLUTIONS - solutions.size(); rawSolutions.size() > maxSize) rawSolutions.resize(maxSize);

            solutions.insert(solutions.end(), rawSolutions.begin(), rawSolutions.end());
        }
    }

    ranges::sort(solutions, isSmaller);

    return solutions;
}


}
