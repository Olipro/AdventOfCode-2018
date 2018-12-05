#include <algorithm>
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
		return id != other.id && (left < other.right && right > other.left && top < other.bottom && bottom > other.top);
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
	auto i = std::remove_if(claims.begin(), claims.end(), [&claims] (const auto& claim) {
		for (const auto& other : claims) {
			if (claim.Overlaps(other))
				return true;

		}
		return false;
	});
	claims.erase(i, claims.end());
	std::cout << claims.front().ID() << std::endl;
}

