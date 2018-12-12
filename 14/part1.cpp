#include <iostream>
#include <string>
#include <utility>
#include <vector>

class Recipe {
	uint8_t score;
public:
	Recipe(uint8_t score) noexcept : score{score} {}
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
		board.reserve(1'000'000);
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

	void FindTenAfter(size_t size) {
		while (board.size() < (size + 10))
			DoRound();
		auto i = board.begin() + size - 1;
		for (auto x = 0; x < 10; ++x)
			std::cout << (++i)->Score();
		std::cout << '\n';
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
	auto rounds = std::stoi(argv[1]);
	std::cout << "Doing " << rounds << " rounds.\n";
	RecipeBoard rb;
	rb.FindTenAfter(rounds);
}
