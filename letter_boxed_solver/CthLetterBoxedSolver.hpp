#pragma once

#include <algorithm>
#include <array>
#include<thread>
#include <vector>

#include "CthUtils.hpp"


namespace cth {

void prepareWordList(std::filesystem::path file);
vector<vector<string>> solve(std::filesystem::path const& path, std::string_view valid_chars);
}
