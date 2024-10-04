#include <iostream>
#include <string>

#include "CthLetterBoxedSolver.hpp"
#include "CthUtils.hpp"

#include <execution>
#include <print>
#include <ranges>

#include <cth/io/log.hpp>


namespace cth {


void prepareWordList(std::filesystem::path file) {
    vector<string> content = loadTextFile(file.generic_string(), LIST_DELIMITER);

    std::sort(std::execution::par_unseq, content.begin(), content.end());

    CTH_STABLE_ABORT(content.size() < 20, "size < 20, probably wrong word list"){}

    file.replace_extension(".cth");

    vector<uint8_t> optimized(content.size() * ROW_SIZE);
    size_t index = 0;

    for(auto& line : content) {
        if(line.size() <= 2) continue;
        CTH_STABLE_ABORT(line.size() > MAX_WORD_SIZE, "word too long, consider increasing the row size") {}

        optimized[index] = static_cast<uint8_t>(line.size());
        for(size_t k = 0; k < line.size(); k++) {
            if(line[k] >= 'A' && line[k] <= 'Z') line[k] += 'a' - 'A';

            optimized[index + k + 1] = line[k] - 'a';
        }

        index += ROW_SIZE;
    }

    optimized.resize(index);

    std::ofstream out{file, std::ios::binary | std::ios::trunc};

    out.write(reinterpret_cast<char const*>(optimized.data()), static_cast<streamsize>(optimized.size()));
    out.close();
}


vector<uint8_t> loadDictionary(std::filesystem::path const& path, std::span<uint8_t const> valid_chars) {
    vector<uint8_t> dictionary{};

    auto const list = loadWordList(path);

    std::array<uint8_t, 26> validChars{};

    for(size_t i = 0; i < valid_chars.size(); ++i)
        validChars[valid_chars[i]] = static_cast<uint8_t>(i / CHARS_PER_SIDE + 1);


    auto const rows = std::ranges::chunk_view{list, ROW_SIZE};
    for(auto const row : rows) {
        auto const size = static_cast<size_t>(row[0]);
        size_t prev = 0;
        for(size_t i = 0; i < size; ++i) {
            auto const side = validChars[row[i + 1]];
            if(side == 0 || side == prev) goto next;
            prev = side;
        }

        dictionary.insert_range(dictionary.end(), row);
    next:;
    }

    return dictionary;
}

std::array<size_t, 27> calcDictionaryEntries(std::span<uint8_t const> sorted_valid_chars, std::span<uint8_t const> dictionary) {
    std::array<size_t, 27> entries{};

    size_t index = 0;

    for(size_t i = 0; i < dictionary.size(); i += ROW_SIZE) {
        if(dictionary[i + 1] == sorted_valid_chars[index]) continue;

        entries[sorted_valid_chars[index] + 1] = i;
        ++index;
        entries[sorted_valid_chars[index]] = i;
    }
    entries[sorted_valid_chars.back() + 1] = dictionary.size();
    return entries;
}


vector<vector<string>> solve(std::filesystem::path const& path, std::string_view valid_chars) {
    CTH_CRITICAL(path.extension() != ".cth", "must be an optimized word list"){}

    static constexpr size_t NO_INDEX = std::numeric_limits<size_t>::max();

    vector<std::array<size_t, MAX_WORDS>> solutions{};
    std::vector<uint8_t> chars(valid_chars.size());
    std::ranges::transform(valid_chars, chars.begin(), [](char c) { return static_cast<uint8_t>(c - 'a'); });

    auto const dictionary = loadDictionary(path, chars);
    std::ranges::sort(chars);
    auto const entries = calcDictionaryEntries(chars, dictionary);

    auto const firstChar = [&dictionary](size_t row_begin) { return dictionary[row_begin + 1]; };
    auto const wordSize = [&dictionary](size_t row_begin) { return dictionary[row_begin]; };
    auto const lastChar = [wordSize, &dictionary](size_t row_begin) { return dictionary[row_begin + wordSize(row_begin)]; };

    auto const begin = [&entries](uint8_t character) { return entries[character]; };
    auto const end = [&entries](uint8_t character) { return entries[character + 1]; };


    auto const modify = [wordSize, &dictionary]<class T>(std::span<uint8_t, 26> counter, size_t row_begin, T) {
        static constexpr T OP{};
        CTH_CRITICAL(row_begin > dictionary.size(), "invalid row begin") {}

        auto const wordBegin = row_begin + 1;
        auto const rowEnd = wordBegin + wordSize(row_begin);
        for(size_t k = wordBegin; k != rowEnd; ++k) {
            auto& num = counter[dictionary[k]];

            num = static_cast<uint8_t>(OP(num, 1ui8));
        }
    };

    for(size_t i = 0; i < dictionary.size(); i += ROW_SIZE) {
        std::vector<std::array<size_t, MAX_WORDS>> partialSolutions{};

        std::array<size_t, MAX_WORDS - 1> rowBegins{};
        rowBegins.fill(NO_INDEX);

        std::array<uint8_t, 26> charCounter{};
        modify(charCounter, i, std::plus{});


        auto const setBegin = [modify, &rowBegins, &charCounter](size_t row_index, size_t new_row_begin) {
            rowBegins[row_index] = new_row_begin;
            modify(charCounter, new_row_begin, std::plus{});
        };

        auto const next = [end, setBegin, begin, lastChar, firstChar, &modify, &rowBegins, &charCounter](size_t row_index) {
            CTH_CRITICAL(row_index >= rowBegins.size(), "row index invalid") {}


            auto& rowBegin = rowBegins[row_index];

            modify(charCounter, rowBegin, std::minus{});

            auto const newBegin = rowBegin + ROW_SIZE;
            auto const rowEnd = end(firstChar(rowBegin));
            CTH_CRITICAL(newBegin > rowEnd, "must be less or equal") {}
            if(newBegin == rowEnd) {
                rowBegin = NO_INDEX;
                return;
            }

            setBegin(row_index, newBegin);

            if(row_index == rowBegins.size() - 1) return;
            CTH_CRITICAL(rowBegins[row_index + 1] != NO_INDEX, "must be empty to increment") {}

            setBegin(row_index + 1, begin(lastChar(newBegin)));
        };

        setBegin(0, begin(lastChar(i)));

        while(true) {
            if(std::ranges::none_of(chars, [&charCounter](auto const c) { return charCounter[c] == 0; })) {
                partialSolutions.emplace_back();
                partialSolutions.back()[0] = i;
                std::ranges::copy(rowBegins, partialSolutions.back().begin() + 1);
            }

            int j = rowBegins.size() - 1;
            while(j >= 0 && rowBegins[j] == NO_INDEX) --j;
            if(j < 0) break;
            next(j);
        }

        solutions.append_range(partialSolutions);
    }


    std::vector<std::vector<string>> result{};
    result.resize(solutions.size());

    for(auto [src, dst] : std::views::zip(solutions, result))
        for(auto const first : src) {
            if(first == NO_INDEX) break;
            std::ranges::subrange const word{dictionary.begin() + first + 1, dictionary.begin() + first + wordSize(first)};
            dst.emplace_back(std::from_range, word | std::views::transform([](uint8_t character) { return character + 'a'; }));
        }

    std::sort(std::execution::par_unseq, result.begin(), result.end(), [](auto const& a, auto const& b) { return a.size() < b.size(); });

    return result;

}
}
