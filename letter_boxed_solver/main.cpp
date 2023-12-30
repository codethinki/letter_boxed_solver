#include <iostream>
#include <Windows.h>

#include "CthLetterBoxedSolver.hpp"

//#define PREPARE_WORD_LIST 1
#undef PREPARE_WORD_LIST

namespace cth {
using namespace std;

}


int main() {
    using namespace std;
    using namespace cth;

    const array<string, 2> wordlistPaths = { "words_easy.txt", "words_hard.txt"};

#ifdef PREPARE_WORD_LIST
	prepareWordList(wordlistPath);
#endif
    const string sideChars = getLetterBoxedSides();
    const string sortedSideChars = [sideChars]() {
        string x = sideChars;
        ranges::sort(x);
        return x;
    }();;

    const auto start = chrono::high_resolution_clock::now();
    vector<string> solutions{};

    for(int i = 0; i < wordlistPaths.size() && solutions.size() < MIN_SOLUTIONS; i++) {
        const auto dictionary = loadDictionaryFromWordlist(wordlistPaths[i], sideChars);
        solutions = calcBestSolutions(sortedSideChars, dictionary); //discard previous solutions since words_easy is a subset of words_hard
    }

    const float time = chrono::duration<float>(chrono::high_resolution_clock::now() - start).count();

    if(solutions.empty()) {
        cout << "\nfound no solutions :(\n";
        return EXIT_FAILURE;
    }
    if(solutions.size() < MIN_SOLUTIONS) cout << "\nfound only " << to_string(solutions.size()) << " solutions :(\n";


    cout << '\n';
    for(int i = solutions.size() - 1; i >= 1; --i) cout << solutions[i] << '\n';

    cout << '\n' << "(good) solution: " << solutions[0] << '\n';
    cout << "\n solutions: " << to_string(solutions.size()) << " [min: " << MIN_SOLUTIONS << ", max: " << MAX_SOLUTIONS << "]\n";
    cout << " took: " << time << 's' << endl;

    system("pause");

    return EXIT_SUCCESS;
}
