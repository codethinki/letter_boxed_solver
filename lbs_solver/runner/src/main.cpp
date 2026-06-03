#include "solver/solver.hpp"

#include <cth/data/cxpr.hpp>
#include <cth/win/io.hpp>

#include <chrono>
#include <print>


namespace lbs {
using clock = std::chrono::high_resolution_clock;
using fms = std::chrono::duration<double, std::milli>;
struct word_list_t {
    std::string_view csv;
    std::vector<size_t> splits;
};



word_list_t dyn_load(std::string_view word_list_path) {
    auto const startLoad = clock::now();
    auto const mapping = cth::win::io::map_file(word_list_path);
    auto const data = mapping.data;
    auto const endLoad = clock::now();

    auto const startSplit = clock::now();
    std::string_view csv{reinterpret_cast<char const*>(data.data()), data.size()};
    auto splits = lbs::split(csv, ',');
    auto const endSplit = clock::now();

    std::println("loaded data in {}", fms{endLoad - startLoad});
    std::println("split words in {}", fms{endSplit - startSplit});
    return {csv, std::move(splits)};
}

constexpr word_list_t precompute_word_list(std::string_view csv) {
    return {csv, split(csv, ',')};
}

void solve_timed(std::string_view characters, std::string_view csv, std::span<size_t const> splits) {
    auto const startFiltering = clock::now();
    auto const compressionIndex = gen_compression_index(characters);
    auto const& [words, wordHashes] = filter_and_hash(
        compressionIndex,
        csv,
        splits
    );
    auto const endFiltering = clock::now();

    auto const startSolve = clock::now();
    auto const solutions = lbs::solve(compressionIndex, words, wordHashes);
    auto const endSolve = clock::now();


    std::println("filtered & hashed in {}", fms{endFiltering - startFiltering});
    std::println("solved in {}", fms{endSolve - startSolve});

    for(auto const& solution : solutions)
        std::println("{}", solution);
}
}


#define DYNAMIC_WORD_LIST_PATH_DEF "assets/words_easy.txt"
#define STATIC_WORD_LIST_PATH_DEF "../assets/words_easy.txt"


namespace lbs {


namespace dev {
    constexpr char raw_word_list[] = {
#embed STATIC_WORD_LIST_PATH_DEF
    };
}


constexpr std::string_view WORD_LIST_CSV{dev::raw_word_list, sizeof(dev::raw_word_list)};

constexpr std::string_view WORD_LIST_PATH{DYNAMIC_WORD_LIST_PATH_DEF};

void solve_manual(std::string_view characters) {
    //auto [csv, splits] = lbs::dyn_load(WORD_LIST_PATH);
    constexpr auto splits = cth::dt::as_cxpr_array<[] { return lbs::split(WORD_LIST_CSV, ','); }>();


    lbs::solve_timed(characters, WORD_LIST_CSV, splits);

}
}


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


//void solve_gpt(std::string_view, std::string_view);

int main() {
    auto sides = read_sides();

    lbs::solve_manual(sides);

    std::println();
    //solve_gpt(sides, DYNAMIC_WORD_LIST_PATH_DEF);
}

//#include "gpt_runner.hpp"
//void solve_gpt(std::string_view sides, std::string_view dictionary) {
//    LetterBoxedSolver solver;
//
//    // Example layout. Letters represent the 4 sides: [A, R, T], [E, O, I], [S, P, C], [U, N, L]
//
//    auto start_time = std::chrono::high_resolution_clock::now();
//    std::vector<std::string> solution = solver.run(sides, dictionary);
//    auto end_time = std::chrono::high_resolution_clock::now();
//
//    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
//
//    if(!solution.empty()) {
//        std::cout << "Solution Found:\n";
//        for(const auto& word : solution) {
//            std::cout << "  -> " << word << "\n";
//        }
//    }
//    else {
//        std::cout << "No solution found.\n";
//    }
//
//    std::cout << "\nExecution Time: " << (duration / 1000.0) << " ms\n";
//}
