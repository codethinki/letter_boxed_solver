#pragma once
#include "solver/settings.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <random>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace lbs::bench {

/**
 * generates puzzles, every 3 consecutive letters of a puzzle form a side
 */
class puzzle_generator {
public:
    puzzle_generator(std::span<std::string_view const> words, unsigned seed) : _rng{seed} {
        for(auto const word : words)
            if(playable(word))
                _byFirstLetter[word.front() - 'a'].emplace_back(word, letter_mask(word));

        for(auto const& candidates : _byFirstLetter)
            _playable.append_range(candidates);
    }

    /**
     * 12 distinct random letters with at least `min_vowels` vowels, often unsolvable
     */
    [[nodiscard]] std::string random(size_t min_vowels = 3) {
        static constexpr std::string_view VOWELS = "aeiou";

        std::string alphabet = "abcdefghijklmnopqrstuvwxyz";
        std::string letters;

        do {
            std::ranges::shuffle(alphabet, _rng);
            letters = alphabet.substr(0, BOX_CHARS);
        }
        while(static_cast<size_t>(std::ranges::count_if(letters, [](char c) { return VOWELS.contains(c); })) < min_vowels);

        return letters;
    }

    /**
     * the letters of two chained words with 12 distinct letters, placed so that both words are valid
     * @details always solvable, like the daily nyt puzzles
     */
    [[nodiscard]] std::string constructed() {
        while(true) {
            auto const& [first, firstMask] = pick(_playable);
            auto const& candidates = _byFirstLetter[first.back() - 'a'];
            if(candidates.empty())
                continue;

            auto const& [second, secondMask] = pick(candidates);
            if(std::popcount(firstMask | secondMask) != static_cast<int>(BOX_CHARS))
                continue;

            if(auto puzzle = place_on_sides(firstMask | secondMask, {first, second}))
                return *std::move(puzzle);
        }
    }

private:
    using word_entry = std::pair<std::string_view, uint32_t>;

    [[nodiscard]] static uint32_t letter_mask(std::string_view word) {
        uint32_t mask = 0;
        for(char const c : word)
            mask |= 1u << (c - 'a');
        return mask;
    }

    [[nodiscard]] static bool playable(std::string_view word) {
        if(word.size() < 3 || !std::ranges::all_of(word, [](char c) { return c >= 'a' && c <= 'z'; }))
            return false;

        return std::ranges::adjacent_find(word) == word.end()
            && std::popcount(letter_mask(word)) <= static_cast<int>(BOX_CHARS);
    }

    [[nodiscard]] word_entry const& pick(std::span<word_entry const> entries) {
        std::uniform_int_distribution<size_t> dist{0, entries.size() - 1};
        return entries[dist(_rng)];
    }

    /**
     * assigns the letters of `mask` to the sides so that no two consecutive letters of `words` share a side
     */
    [[nodiscard]] std::optional<std::string> place_on_sides(uint32_t mask, std::array<std::string_view, 2> words) {
        std::array<std::array<bool, 26>, 26> adjacent{};
        for(auto const word : words)
            for(size_t i = 1; i < word.size(); i++) {
                adjacent[word[i - 1] - 'a'][word[i] - 'a'] = true;
                adjacent[word[i] - 'a'][word[i - 1] - 'a'] = true;
            }

        std::vector<int> letters{};
        for(int c = 0; c < 26; c++)
            if(mask & (1u << c))
                letters.push_back(c);
        std::ranges::shuffle(letters, _rng);

        std::array<int, 26> sideOf{};
        sideOf.fill(-1);
        std::array<size_t, SIDES> sideLoad{};

        auto const place = [&](this auto const& self, size_t i) -> bool {
            if(i == letters.size())
                return true;

            int const letter = letters[i];
            for(size_t side = 0; side < SIDES; side++) {
                if(sideLoad[side] == CHARS_PER_SIDE)
                    continue;

                bool const blocked = std::ranges::any_of(
                    letters,
                    [&](int other) { return adjacent[letter][other] && sideOf[other] == static_cast<int>(side); }
                );
                if(blocked)
                    continue;

                sideOf[letter] = static_cast<int>(side);
                ++sideLoad[side];
                if(self(i + 1))
                    return true;
                sideOf[letter] = -1;
                --sideLoad[side];
            }
            return false;
        };

        if(!place(0))
            return std::nullopt;

        std::string puzzle{};
        for(size_t side = 0; side < SIDES; side++)
            for(int const letter : letters)
                if(sideOf[letter] == static_cast<int>(side))
                    puzzle.push_back(static_cast<char>('a' + letter));
        return puzzle;
    }

    std::mt19937 _rng;
    std::array<std::vector<word_entry>, 26> _byFirstLetter{};
    std::vector<word_entry> _playable{};
};

}
