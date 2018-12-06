#include <array>
#include <bitset>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
#include <unordered_set>

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
		return id != other.id && !(left > other.right || right < other.left || top > other.bottom || bottom < other.top);
	}
};

class OverlapCounter {
	uint32_t nonOverlappingID = 0;

public:
	OverlapCounter(std::istream& in) {
		std::vector<Claim> claims;
		claims.reserve(1500);
		while (in.get() == '#') {
			claims.emplace_back(in);
			in.seekg(1, std::ios_base::cur);
		}
		for (auto&& claimA : claims) {
			bool overlaps = false;
			for (auto&& claimB : claims) {
				if (claimA.Overlaps(claimB)) {
					overlaps = true;
					break;
				}
			}
			if (!overlaps) {
				nonOverlappingID = claimA.ID();
				return;
			}
		}
	}

	uint32_t NonOverlappingID() const {
		return nonOverlappingID;
	}
};

int main(int argc, char* argv[]) {
	std::ifstream file{argc == 2 ? argv[1] : "input.txt"};
	OverlapCounter overlapCounter{file};
	std::cout << overlapCounter.NonOverlappingID() << std::endl;
}

