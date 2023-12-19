#include <algorithm>
#include <array>
#include <assert.h>
#include<fstream>
#include <iostream>
#include <span>
#include<thread>
#include <omp.h>
#include <unordered_map>
#include <vector>

using namespace std;

vector<string> loadTextFile(const string_view path) {
	vector<string> data{};
	fstream file(path.data());
	string line;
	while (getline(file, line)) data.push_back(line);

	file.close();
	return data;
}
template<typename RandIt>
void writeTextFile(const string_view filepath, RandIt data_begin, const RandIt& data_end, const char delimiter = '\n') {
	ofstream file(filepath.data(), ios::ate);

	while (data_begin != data_end) {
		file << *data_begin << delimiter;
		++data_begin;
	}
	file.close();
}
void prepareWordList(const string_view path) {
	vector<string> content = loadTextFile(path);
	vector<string> wordlist{};

	vector<uint32_t> removable{};
#pragma omp parallel for
	for (int i = 0; i < content.size(); i++) {
		string& line = content[i];
		bool valid = line.size() >= 3;

		for (int k = 0; valid && k < line.size(); k++) {
			char& c = line[k];
			if (c >= 'a' && c <= 'z') continue;
			if (c >= 'A' && c <= 'Z') {
				c += 'a' - 'A';
				continue;
			}
			valid = false;
		}
#pragma omp critical
		if (!valid) removable.push_back(i);
	}
	ranges::sort(removable);
	for (int i = 0; i < removable.size(); i++) content.erase(content.begin() + removable[i] - i);

	ranges::sort(content);

	writeTextFile(path, content.begin(), content.end(), ',');
}



vector<string> loadDictionaryFromWordlist(const string_view dictionary_path, const string_view valid_chars) {
	constexpr static uint32_t ROW_SIZE = 3;

	vector<string> words{};
	ifstream file(dictionary_path.data());

	if (!file.is_open()) {
		cout << "ERROR: no dictionary found";
		exit(EXIT_FAILURE);
	}

	bool finished = false;
#pragma omp parallel shared(finished)
	{

		string line;
		vector<string> validWords{};
		while (!finished) {
#pragma omp critical
			finished = !static_cast<bool>(getline(file, line, ','));

			size_t prevSide = valid_chars.find(line[0]);
			for (int i = 1; prevSide != string::npos && i < line.size(); i++) {
				const size_t side = valid_chars.find(line[i]);

				prevSide = (prevSide / ROW_SIZE == side / ROW_SIZE) ? string::npos : side;
			}
			if (prevSide == string::npos) continue;

			validWords.push_back(line);
		}

#pragma omp critical
		words.insert(words.end(), validWords.begin(), validWords.end());
	}
	ranges::sort(words);

	file.close();

	return words;
}

/**
 * \brief increments the correct offset value for the next word
 * \param offsets offset in validWords list
 * \return index of the offset that needs to be updated .> offsets.size() = no update required
 */
inline uint32_t nextOffset(vector<pair<uint32_t, uint32_t>>& offsets) {
	//offset[0] is fixed
	for (int i = offsets.size() - 1; i >= 1; --i) {
		offsets[i].first += 1;
		if (offsets[i].first < offsets[i].second) return i + 1;
	}
	return 0;
}
inline void nextOffsetSet(const int first_update, vector<pair<uint32_t, uint32_t>>& offsets, const vector<string_view>& dictionary,
	unordered_map<char, uint32_t> char_map, const array<uint32_t, 13>& first_char_map) {
	assert(first_update > 0 && "first update cannot be < 1");
	for (int i = first_update; i < offsets.size(); i++) {
		const int lastCharIndex = char_map[dictionary[offsets[i - 1].first].back()];
		offsets[i].first = first_char_map[lastCharIndex];
		offsets[i].second = first_char_map[lastCharIndex + 1];
	}

}

vector<string> calcFixedSizeSolutions(const int word_count, const string_view sorted_valid_chars, const vector<string_view>& dictionary) {
	vector<string> solutions{};

	unordered_map<char, uint32_t> charMap{};
	for (int i = 0; i < sorted_valid_chars.size(); i++) charMap[sorted_valid_chars[i]] = i;

	const array<uint32_t, 13> firstCharMap = [sorted_valid_chars, dictionary]() {
		array<uint32_t, 13> arr{};
		for (int i = 0; i < sorted_valid_chars.size(); i++) {
			int k = 0;
			for (; k < dictionary.size() && dictionary[k][0] != sorted_valid_chars[i]; k++);

			arr[i] = k;
		}

		arr.back() = dictionary.size();

		return arr;
		}();

#pragma omp parallel for shared(charMap, firstCharMap)
		for (int i = 0; i < dictionary.size(); i++) {
			vector<pair<uint32_t, uint32_t>> offsets(word_count);
			//set the first offset based on the current fixed word
			offsets[0].first = i;
			offsets[0].second = dictionary.size();

			int firstSetUpdate = 1;
			while (firstSetUpdate) {
				nextOffsetSet(firstSetUpdate, offsets, dictionary, charMap, firstCharMap);

				string solution = "";
				for (int k = 0; k < offsets.size(); k++) solution += dictionary[offsets[k].first];
				bool valid = true;
				for (int k = 0; valid && k < sorted_valid_chars.size(); k++) valid = solution.contains(sorted_valid_chars[k]);

				if (valid) {
					solution = "";
					for (int k = 0; k < offsets.size(); k++) {
						solution += dictionary[offsets[k].first];
						solution += ", ";
					}
#pragma omp critical
					solutions.push_back(solution.substr(0, solution.size() - 2));
				}

				firstSetUpdate = nextOffset(offsets);
			}
		}
		return solutions;
}

vector<string> calcBestSolutions(const string_view sorted_valid_chars, const vector<string_view>& dictionary_view) {
	vector<string> solutions{};
	for (int i = 1; solutions.empty() && i < 6; i++) solutions = calcFixedSizeSolutions(i, sorted_valid_chars, dictionary_view);

	ranges::sort(solutions, [](const string_view a, const string_view b) { return a.size() > b.size(); });

	if (solutions.size() > 30) solutions.resize(30);

	return solutions;
}



string getValidChars() {
	string validChars{};

	cout << "input the characters of one rectangle each (lowercase):\n";
	for (int i = 0; i < 4; i++) {
		cout << "side [" << to_string(i + 1) << "]: ";
		bool valid = true;
		string input;
		do {
			if (!valid) cout << "  invalid, try again: ";
			cin >> input;
			valid = input.size() == 3;
			for (int k = 0; k < 3 && valid; k++) valid = input[k] >= 'a' && input[k] <= 'z' && !validChars.contains(input[k]);
		} while (!valid);
		validChars += input;
	}

	return validChars;
}

int main() {
	const string wordlistPath = "words_hard.txt";

	//prepareWordList(wordlistPath);

	string validChars = getValidChars();

	const auto start = chrono::high_resolution_clock::now();

	auto dictionary = loadDictionaryFromWordlist(wordlistPath, validChars);



	ranges::sort(validChars.begin(), validChars.end());

	vector<string_view> dictionaryView{};
	dictionaryView.insert(dictionaryView.begin(), dictionary.begin(), dictionary.end());

	const vector<string> solutions = calcBestSolutions(validChars, dictionaryView);

	const float time = chrono::duration<float>(chrono::high_resolution_clock::now() - start).count();

	for (int i = 1; i < solutions.size(); i++) cout << solutions[i] << '\n';

	cout << "\nbest solution: " << solutions.back() << '\n' << "took " << time << "s";

	return EXIT_SUCCESS;
}
