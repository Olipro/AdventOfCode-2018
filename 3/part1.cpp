#include <array>
#include <bitset>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

class Claim {
	uint32_t id;
	uint32_t top;
	uint32_t bottom;
	uint32_t left;
	uint32_t right;
public:
	Claim(std::istream& s) {
		s >> id;
		s.seekg(3, std::ios_base::cur);
		s >> left;
		s.seekg(1, std::ios_base::cur);
		s >> top;
		s.seekg(2, std::ios_base::cur);
		s >> right;
		s.seekg(1, std::ios_base::cur);
		s >> bottom;
		bottom += top;
		right += left;
	}

	uint32_t ID() const {
		return id;
	}
	uint32_t Top() const {
		return top;
	}
	uint32_t Bottom() const {
		return bottom;
	}
	uint32_t Left() const {
		return left;
	}
	uint32_t Right() const {
		return right;
	}

	bool Overlaps(const Claim& other) const {
		return !(left < other.right && right > other.left && top > other.bottom && bottom < other.top);
	}

	uint32_t OverlapQuantity(const Claim& other) const {
		if (id == other.id)
			return 0;
		auto dx = std::min(right, other.right) - std::max(left, other.left);
		auto dy = std::min(bottom, other.bottom) - std::max(top, other.top);
		return dx * dy;
	}

	bool operator<(const Claim& other) const {
		return left < other.left && top < other.top;
	}
};

using Grid = std::array<std::bitset<1000>, 1000>;

void SetGrid(Grid& grid, const Claim& claim) {
	for (auto i = claim.Top(); i < claim.Bottom(); ++i)
		for (auto j = claim.Left(); j < claim.Right(); ++j)
			grid[i].set(j);
}

void ComputeOverlap(Grid& grid, const Grid& other, const Claim& box) {
	for (auto i = box.Top(); i < box.Bottom(); ++i)
		grid[i] &= other[i];
}

void AssignOverlap(Grid& grid, const Grid& other, const Claim& box) {
	for (auto i = box.Top(); i < box.Bottom(); ++i)
		grid[i] |= other[i];
}

int main(int argc, char* argv[]) {
	std::ifstream file{argc == 2 ? argv[1] : "input.txt"};
	std::vector<Claim> claims;
	while (file.get() == '#') {
		claims.emplace_back(file);
		file.seekg(1, std::ios_base::cur);
	}
	Grid grid;
	for (auto iter = claims.rbegin(); iter != claims.rend(); ++iter) {
		auto a = std::move(*iter);
		claims.pop_back();
		Grid gridA;
		SetGrid(gridA, a);
		for (const auto& b : claims) {
			if (!a.Overlaps(b))
				continue;
			Grid gridB;
			SetGrid(gridB, b);
			ComputeOverlap(gridB, gridA, b);
			AssignOverlap(grid, gridB, b);
		}
	}
	auto total = 0;
	for (auto& row : grid)
		total += row.count();
	std::cout << total << std::endl;
}

