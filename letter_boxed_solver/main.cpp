#include "CthLetterBoxedSolver.hpp"

#undef PREPARE_WORD_LIST




int main() {
    using namespace std;
    using namespace cth;

    const string wordlistPath = "words_hard.txt";

#ifdef PREPARE_WORD_LIST
	prepareWordList(wordlistPath);
#endif

    string letterBoxedSides = "rinmscdtugoa"; // getLetterBoxedSides();

    const auto start = chrono::high_resolution_clock::now();

    const auto dictionary = loadDictionaryFromWordlist(wordlistPath, letterBoxedSides);

    ranges::sort(letterBoxedSides);

    const vector<string> solutions = calcBestSolutions(letterBoxedSides, dictionary);

    const float time = chrono::duration<float>(chrono::high_resolution_clock::now() - start).count();

    if(solutions.empty()) {
        cout << "\nfound no solutions :(\n";
        return EXIT_FAILURE;
    }

    cout << '\n';
    for(int i = solutions.size() - 1; i >= 0; --i) cout << solutions[i] << '\n';

    //evaluate the best solution based on the least repetitions of a single char in a solutions[i] string in a lambda function


    cout << "\nbest solution: " << solutions[0] << '\n' << "\ntook " << time << "s";

    return EXIT_SUCCESS;
}
