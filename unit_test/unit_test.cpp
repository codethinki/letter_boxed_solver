#include "pch.h"
#include "CppUnitTest.h"
#include "../letter_boxed_solver/CthLetterBoxedSolver.hpp"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace unittest {
TEST_CLASS(unittest) {
public:
    TEST_METHOD(load_dictionary_old) {
        using namespace std;
		using namespace cth;

		const string wordlistPath = "words_hard.txt";

		const auto dictionary = loadDictionaryFromWordlistOld(wordlistPath, "rinmscdtugoa");

		Assert::IsTrue(dictionary.size());
    }
	TEST_METHOD(load_dictionary) {
        using namespace std;
		using namespace cth;

		const string wordlistPath = "words_hard.txt";
		const auto dictionary = loadDictionaryFromWordlist(wordlistPath, "rinmscdtugoa");
		Assert::IsTrue(dictionary.size());
    }
};
}
