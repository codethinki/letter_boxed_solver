#include "solver/solver.hpp"

#include <cth/data/cxpr.hpp>
#include <cth/io/file.hpp>

#include <print>


namespace lbs {
struct word_list_t {
    std::string_view csv;
    std::vector<size_t> wordEnds;
    std::vector<std::byte> data{};
};



word_list_t dyn_load(std::string_view word_list_path) {
    auto data = cth::io::file::read<std::byte>(word_list_path);
    std::string_view csv{reinterpret_cast<char const*>(data.data()), data.size()};
    auto wordEnds = lbs::word_ends(csv, ',');
    return {csv, std::move(wordEnds), std::move(data)};
}

void print_solutions(std::string_view characters, std::string_view csv, std::span<size_t const> wordEnds) {
    auto const compressionIndex = gen_compression_index(characters);
    auto const& [words, wordHashes] = filter_and_hash(
        compressionIndex,
        csv,
        wordEnds
    );

    auto const solutions = lbs::solve(compressionIndex, words, wordHashes);
    if(solutions.empty())
        std::println("no solution");

    for(auto const& solution : solutions)
        std::println("{}", solution);
}
}


#define DYNAMIC_WORD_LIST_PATH_DEF "assets/words_easy.txt"
#define STATIC_WORD_LIST_PATH_DEF "../assets/words_easy.txt"


namespace {
#ifdef __has_embed
    namespace dev {
        constexpr char raw_word_list[] = {
#embed STATIC_WORD_LIST_PATH_DEF
        };
    }

    constexpr std::string_view WORD_LIST_CSV{dev::raw_word_list, sizeof(dev::raw_word_list)};

    void solve_manual(std::string_view characters) {
        constexpr auto wordEnds = cth::dt::as_cxpr_array<[] { return lbs::word_ends(WORD_LIST_CSV, ','); }>();

        lbs::print_solutions(characters, WORD_LIST_CSV, wordEnds);
    }
#else
    constexpr std::string_view WORD_LIST_PATH{DYNAMIC_WORD_LIST_PATH_DEF};

    void solve_manual(std::string_view characters) {
        auto const wordList = lbs::dyn_load(WORD_LIST_PATH);

        lbs::print_solutions(characters, wordList.csv, wordList.wordEnds);
    }
#endif



    [[nodiscard]] std::string read_sides() {
        std::vector<std::string> sides{};
        sides.reserve(lbs::SIDES);

        while(true) {
            for(size_t i = 0; i < lbs::SIDES; i++) {
                std::println("enter side {}", i);
                std::cin >> sides.emplace_back();

                if(sides.back().size() != lbs::CHARS_PER_SIDE) {
                    sides.clear();
                    break;
                }
            }

            if(sides.size() == lbs::SIDES)
                return {std::from_range, sides | std::views::join};

            sides.clear();
            std::println("invalid, try again");
        }
    }


    void pause() {
        std::cout << "Press Enter to continue . . ." << std::flush;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
    }
}


int main() {
    auto const sides = read_sides();

    solve_manual(sides);

    std::println();
    pause();
}
