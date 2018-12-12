#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

class Recipe {
	uint8_t score;
public:
	Recipe(uint8_t score) noexcept : score{score} {}
	bool operator==(const uint8_t val) const noexcept {
		return score == val;
	}
	bool operator==(const Recipe& other) const noexcept {
		return score == other.score;
	}
	uint16_t Score() const noexcept {
		return score;
	}
};

class RecipeBoard {
	std::vector<Recipe> board{{3}, {7}};
	size_t elf1 = 0;
	size_t elf2 = 1;
public:
	RecipeBoard() {
		board.reserve(30'000'000); //reserve the memory in advance to avoid realloc
	}
	void DoRound() {
		auto next = board[elf1].Score() + board[elf2].Score();
		if (next < 10)
			board.emplace_back(next);
		else {
			auto rmdr = next % 10;
			board.emplace_back(next / 10);
			board.emplace_back(rmdr);
		}
		elf1 = (elf1 + (board[elf1].Score() + 1)) % board.size();
		elf2 = (elf2 + (board[elf2].Score() + 1)) % board.size();
	}

	void FindCountBefore(const std::vector<uint8_t>&  pattern) {
		while (board.size() < pattern.size())
			DoRound(); //Warmup.
		auto i = std::search(board.begin(), board.end(), pattern.begin(), pattern.end());
		for (; i == board.end(); i = std::search(board.end() - (pattern.size() + 1), board.end(), pattern.begin(), pattern.end())) {
			DoRound();
		}
		std::cout << std::distance(board.begin(), i) << '\n';;
	}

	const decltype(board)& Board() const noexcept {
		return board;
	}
};

int main(int argc, const char* argv[]) {
	if (argc != 2) {
		std::cout << "Puzzle input must be provided. e.g. ./part1 12345";
		return 1;
	}
	std::vector<uint8_t> pattern;
	for (auto c = argv[1]; *c; ++c)
		pattern.emplace_back(*c - 0x30);
	std::cout << "Getting count for: ";
	for (const auto& c : pattern)
		std::cout << static_cast<int>(c);
	std::cout << '\n';
	RecipeBoard rb;
	rb.FindCountBefore(pattern);
}
