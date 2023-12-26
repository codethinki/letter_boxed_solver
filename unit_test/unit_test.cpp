#include <pch.h>
#include "CppUnitTest.h"

#include <fstream>
#include <Windows.h>

#include "../letter_boxed_solver/CthLetterBoxedSolver.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace test {
using namespace std;
using namespace cth;

TEST_CLASS(test_solver) {
public:
    string mainProjectPath = "../../letter_boxed_solver/";
    string wordlistPath = mainProjectPath + "words_hard.txt";
    TEST_METHOD(load_dictionary) {
        using namespace std;
        using namespace cth;

        const auto dictionary = loadDictionaryFromWordlist(wordlistPath, "rinmscdtugoa");
        Assert::IsTrue(dictionary.size());
    }
};
}
