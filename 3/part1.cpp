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

class OverlapCoord {
	int x;
	int y;
public:
	OverlapCoord(int x, int y) : x{x}, y{y} {}

	bool operator==(const OverlapCoord& other) const noexcept {
		return x == other.x && y == other.y;
	}

	struct OverlapHash {
		size_t operator()(const OverlapCoord& o) const noexcept {
			return (o.x << 16) | o.y;
		}
	};
};

class OverlapCounter {
	std::unordered_set<OverlapCoord, OverlapCoord::OverlapHash> overlaps{1500};

	void UpdateOverlap(const Claim& a, const Claim& b) {
		for (auto y = a.Top(); y < a.Bottom(); ++y)
			for (auto x = a.Left(); x < a.Right(); ++x) {
				if (x >= b.Left() && x < b.Right() && y >= b.Top() && y < b.Bottom()) {
					overlaps.emplace(x, y);
				}
			}
	}
public:
	OverlapCounter(std::istream& in) {
		std::vector<Claim> claims;
		claims.reserve(1500);
		while (in.get() == '#') {
			claims.emplace_back(in);
			in.seekg(1, std::ios_base::cur);
		}
		for (auto&& claimA : claims) {
			for (auto&& claimB : claims) {
				if (claimA.Overlaps(claimB))
					UpdateOverlap(claimA, claimB);
			}
		}
	}

	size_t OverlapCount() const {
		return overlaps.size();
	}
};

int main(int argc, char* argv[]) {
	std::ifstream file{argc == 2 ? argv[1] : "input.txt"};
	OverlapCounter overlapCounter{file};
	std::cout << overlapCounter.OverlapCount() << std::endl;
}

