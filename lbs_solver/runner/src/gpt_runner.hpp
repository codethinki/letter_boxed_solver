#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <algorithm>
#include <chrono>
#include <windows.h>

class LetterBoxedSolver {
private:
    // Lightweight Windows Memory-Mapped File Wrapper
    struct MappedFile {
        HANDLE file = INVALID_HANDLE_VALUE;
        HANDLE mapping = NULL;
        const char* data = nullptr;
        size_t size = 0;

        bool open(const std::string& path) {
            file = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, 
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (file == INVALID_HANDLE_VALUE) return false;
            
            LARGE_INTEGER fileSize;
            if (!GetFileSizeEx(file, &fileSize)) {
                close();
                return false;
            }
            size = static_cast<size_t>(fileSize.QuadPart);
            
            mapping = CreateFileMappingA(file, NULL, PAGE_READONLY, 0, 0, NULL);
            if (!mapping) {
                close();
                return false;
            }
            
            data = static_cast<const char*>(MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0));
            return data != nullptr;
        }

        void close() {
            if (data) { UnmapViewOfFile(data); data = nullptr; }
            if (mapping) { CloseHandle(mapping); mapping = NULL; }
            if (file != INVALID_HANDLE_VALUE) { CloseHandle(file); file = INVALID_HANDLE_VALUE; }
        }

        ~MappedFile() { close(); }
    };

    struct WordInfo {
        std::string word;
        uint32_t mask;
        char start;
        char end;
    };

    std::vector<WordInfo> valid_words_by_start[26];
    int8_t char_to_side[26];
    std::vector<char> box_letters;
    uint32_t target_mask = 0;

    // Evaluates constraints directly on raw memory boundaries without heap allocations
    inline bool validate_and_process_word(const char* start, const char* end, uint32_t& out_mask) {
        size_t len = end - start;
        if (len < 3) return false; // Game rules: Words must be 3+ letters
        
        uint32_t mask = 0;
        int8_t last_side = -1;
        
        for (const char* p = start; p < end; ++p) {
            char c = *p;
            if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a'; // Standardize to lowercase
            if (c < 'a' || c > 'z') return false;
            
            int8_t side = char_to_side[c - 'a'];
            if (side == -1) return false;           // Letter not in active box
            if (side == last_side) return false;    // Consecutive letters on same side
            
            mask |= (1 << (c - 'a'));
            last_side = side;
        }
        out_mask = mask;
        return true;
    }

    void load_and_filter_dictionary(std::string const& dict_path) {
        MappedFile file;
        if (!file.open(dict_path)) return;

        const char* ptr = file.data;
        const char* end = file.data + file.size;

        while (ptr < end) {
            // Trim / Skip delimiters
            while (ptr < end && (*ptr == '\r' || *ptr == '\n' || *ptr == ',' || *ptr == ' ' || *ptr == '\t')) {
                ptr++;
            }
            if (ptr >= end) break;

            const char* word_start = ptr;
            while (ptr < end && *ptr != '\r' && *ptr != '\n' && *ptr != ',' && *ptr != ' ' && *ptr != '\t') {
                ptr++;
            }
            const char* word_end = ptr;

            uint32_t mask = 0;
            if (validate_and_process_word(word_start, word_end, mask)) {
                char s = *word_start;
                if (s >= 'A' && s <= 'Z') s = s - 'A' + 'a';
                char e = *(word_end - 1);
                if (e >= 'A' && e <= 'Z') e = e - 'A' + 'a';

                valid_words_by_start[s - 'a'].push_back({
                    std::string(word_start, word_end),
                    mask,
                    s,
                    e
                });
            }
        }
    }

    // Graph search using dfs with iterative deepening
    bool dfs(char curr_char, uint32_t current_mask, int depth, int max_depth, 
             std::vector<const WordInfo*>& path, std::vector<std::string>& out_solution) {
        if (current_mask == target_mask) {
            for (auto* w : path) {
                out_solution.push_back(w->word);
            }
            return true;
        }
        if (depth == max_depth) return false;

        int idx = curr_char - 'a';
        for (const auto& w : valid_words_by_start[idx]) {
            path.push_back(&w);
            if (dfs(w.end, current_mask | w.mask, depth + 1, max_depth, path, out_solution)) {
                return true;
            }
            path.pop_back();
        }
        return false;
    }

public:
    std::vector<std::string> run(std::string_view box_chars, std::string_view dictionary_path) {
        // Reset state
        std::fill(std::begin(char_to_side), std::end(char_to_side), -1);
        box_letters.clear();
        target_mask = 0;
        for (int i = 0; i < 26; ++i) valid_words_by_start[i].clear();

        // Parse box layout. Format expects 4 clusters of 3 chars (e.g. "abc def ghi jkl")
        int side_count = 0;
        int char_in_side_count = 0;
        
        for (char c : box_chars) {
            if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
            if (c >= 'a' && c <= 'z') {
                box_letters.push_back(c);
                char_to_side[c - 'a'] = side_count;
                target_mask |= (1 << (c - 'a'));
                char_in_side_count++;
                if (char_in_side_count == 3) {
                    char_in_side_count = 0;
                    side_count++;
                }
            }
        }

        // 1 & 2 & 3. Load, split and filter straight from mapped storage
        load_and_filter_dictionary(std::string{dictionary_path});

        // 4. Solve using iterative deepening
        std::vector<std::string> solution;
        std::vector<const WordInfo*> path;
        for (int max_depth = 1; max_depth <= 3; ++max_depth) {
            for (char start_char : box_letters) {
                int idx = start_char - 'a';
                for (const auto& w : valid_words_by_start[idx]) {
                    path.push_back(&w);
                    if (dfs(w.end, w.mask, 1, max_depth, path, solution)) {
                        return solution;
                    }
                    path.pop_back();
                }
            }
        }
        return {};
    }
};


