#include <iostream>
#include <Windows.h>

#include "CthLetterBoxedSolver.hpp"

namespace cth {
using namespace std;

}


int main() {
    using namespace std;
    using namespace cth;

#ifdef PREPARE_WORD_LIST
	prepareWordList(wordlistPath);
#endif
    const string sideChars = getLetterBoxedSides();

    const auto start = chrono::high_resolution_clock::now();
    const vector<string> solutions = solve(sideChars);
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
