#include <print>

#include <cth/string.hpp>

#include "CthLetterBoxedSolver.hpp"


int main() {
    using namespace std;

    constexpr std::string_view file = "resources/words_hard.txt";

    cth::prepareWordList(file);

    constexpr std::string_view wordList = "resources/words_hard.cth";

  //  string const sideChars = cth::getLetterBoxedSides();

    auto const start = chrono::high_resolution_clock::now();
    auto const solutions = cth::solve(wordList, "hdpnscrijfao");
    float const time = chrono::duration<float>(chrono::high_resolution_clock::now() - start).count();

    if(solutions.empty()) {
        std::println("found no solutions :(");
        return EXIT_FAILURE;
    }
    if(solutions.size() < cth::MIN_SOLUTIONS) std::println("found only {} solutions :(\n", solutions.size());


    std::println();
    for(int i = solutions.size() - 1; i >= 1; --i) std::println("{}", solutions[i]);
    std::println("recommended solution: {}\n\n", solutions[0]);
    std::println(" solutions: {} [min: {}, max: {}]\n", solutions.size(), cth::MIN_SOLUTIONS, cth::MAX_SOLUTIONS);
    std::println("took {}s", time);


    system("pause");

    return EXIT_SUCCESS;
}
