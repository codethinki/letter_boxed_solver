#include "pch.h"

#include <string>

std::string mainProjectPath = "../../letter_boxed_solver/";
std::string wordlistPath = mainProjectPath + "words_hard.txt";

TEST(main_group, main_test) {
	EXPECT_EQ(1, 1);
	EXPECT_TRUE(true);
}