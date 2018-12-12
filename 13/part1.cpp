#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <vector>

enum class TrackDir {
	Vertical,
	Horizontal,
	DiagFwdSlash,
	DiagBackSlash,
	Intersection,
	NONE
};

class TrackPiece {
	uint16_t x;
	uint16_t y;
	TrackDir dir;
public:
	TrackPiece(const uint16_t x, const uint16_t y, const TrackDir dir) noexcept : x{x}, y{y}, dir{dir} {}
	TrackPiece(const uint16_t x, const uint16_t y) noexcept : TrackPiece{x, y, TrackDir::NONE} {}

	bool operator==(const TrackPiece& other) const noexcept {
		return x == other.x && y == other.y;
	}

	uint16_t X() const noexcept {
		return x;
	}

	uint16_t Y() const noexcept {
		return y;
	}

	TrackDir Dir() const noexcept {
		return dir;
	}

	struct Hash {
		size_t operator()(const TrackPiece& self) const noexcept {
			return (self.x << 16) | self.y;
		}
	};
};

enum class NextTurnDir {
	Left,
	Straight,
	Right,
};

class NextTurn {
	NextTurnDir dir = NextTurnDir::Left;
public:
	NextTurnDir Dir() const noexcept {
		return dir;
	}

	NextTurn& operator++() noexcept {
		switch (dir) {
			case NextTurnDir::Left:
				dir = NextTurnDir::Straight;
				break;
			case NextTurnDir::Straight:
				dir = NextTurnDir::Right;
				break;
			case NextTurnDir::Right:
				dir = NextTurnDir::Left;
				break;
		}
		return *this;
	}
};

enum class CartDir {
	Up,
	Down,
	Left,
	Right
};

class CurCartDir {
	CartDir dir;
public:
	CurCartDir(CartDir dir) : dir{dir} {}

	CartDir Dir() const noexcept {
		return dir;
	}

	CurCartDir& TurnLeft() noexcept {
		switch (dir) {
			case CartDir::Up:
				dir = CartDir::Left;
				break;
			case CartDir::Down:
				dir = CartDir::Right;
				break;
			case CartDir::Left:
				dir = CartDir::Down;
				break;
			case CartDir::Right:
				dir = CartDir::Up;
				break;
		}
		return *this;
	}

	CurCartDir& TurnRight() noexcept {
		switch (dir) {
			case CartDir::Up:
				dir = CartDir::Right;
				break;
			case CartDir::Down:
				dir = CartDir::Left;
				break;
			case CartDir::Left:
				dir = CartDir::Up;
				break;
			case CartDir::Right:
				dir = CartDir::Down;
				break;
		}
		return *this;
	}

	bool operator==(const CartDir& dir) const noexcept {
		return this->dir == dir;
	}

	bool operator==(const CurCartDir& other) const noexcept {
		return dir == other.dir;
	}
};

class Cart {
	uint16_t x;
	uint16_t y;
	CurCartDir dir;
	NextTurn nextTurn;

	void DoIntersectionTurn() {
		switch (nextTurn.Dir()) {
			case NextTurnDir::Left:
				dir.TurnLeft();
				break;
			case NextTurnDir::Right:
				dir.TurnRight();
				break;
			case NextTurnDir::Straight:
				break;
		}
		++nextTurn;
		switch (dir.Dir()) {
			case CartDir::Up:
				--y;
				break;
			case CartDir::Down:
				++y;
				break;
			case CartDir::Left:
				--x;
				break;
			case CartDir::Right:
				++x;
				break;
		}
	}
public:
	Cart(const uint16_t x, const uint16_t y, const CartDir dir) : x{x}, y{y}, dir{dir} {}

	void Move(const std::unordered_set<TrackPiece, TrackPiece::Hash>& map) {
		auto i = map.find(TrackPiece{x, y});
		if (i == map.end())
			throw std::logic_error{"track piece not found"};
		auto& track = *i;
		switch (track.Dir()) {
			case  TrackDir::Horizontal:
				x += dir == CartDir::Right ? 1 : -1;
				break;
			case TrackDir::Vertical:
				y += dir == CartDir::Up ? -1 : 1;
				break;
			case TrackDir::DiagFwdSlash:
				if (dir == CartDir::Up) {
					dir = CartDir::Right;
					++x;
				} else if (dir == CartDir::Left) {
					dir = CartDir::Down;
					++y;
				} else if (dir == CartDir::Down) {
					dir = CartDir::Left;
					--x;
				} else {
					dir = CartDir::Up;
					--y;
				}
				break;
			case TrackDir::DiagBackSlash:
				if (dir == CartDir::Up) {
					dir = CartDir::Left;
					--x;
				} else if (dir == CartDir::Left) {
					dir = CartDir::Up;
					--y;
				} else if (dir == CartDir::Down) {
					dir = CartDir::Right;
					++x;
				} else if (dir == CartDir::Right) {
					dir = CartDir::Down;
					++y;
				}
				break;
			case TrackDir::Intersection:
				DoIntersectionTurn();
				break;
			case TrackDir::NONE:
				throw std::logic_error{"Cart on invalid track piece"};
				break;
		}
	}

	bool operator<(const Cart& other) const noexcept {
		return y < other.y || (x < other.x && y == other.y);
	}

	bool operator==(const Cart& other) const noexcept {
		return x == other.x && y == other.y;
	}

	uint16_t X() const noexcept {
		return x;
	}

	uint16_t Y() const noexcept {
		return y;
	}
};

class TrackMap {
	std::unordered_set<TrackPiece, TrackPiece::Hash> map;
	std::vector<Cart> carts;
public:
	TrackMap(std::istream& in) {
		uint16_t x = 0;
		uint16_t y = 0;
		while (in) {
			switch (in.get()) {
				case '/':
					map.emplace(x, y, TrackDir::DiagFwdSlash);
					break;
				case '\\':
					map.emplace(x, y, TrackDir::DiagBackSlash);
					break;
				case '|':
					map.emplace(x, y, TrackDir::Vertical);
					break;
				case '-':
					map.emplace(x, y, TrackDir::Horizontal);
					break;
				case '+':
					map.emplace(x, y, TrackDir::Intersection);
					break;
				case '^':
					map.emplace(x, y, TrackDir::Vertical);
					carts.emplace_back(x, y, CartDir::Up);
					break;
				case 'v':
					map.emplace(x, y, TrackDir::Vertical);
					carts.emplace_back(x, y, CartDir::Down);
					break;
				case '>':
					map.emplace(x, y, TrackDir::Horizontal);
					carts.emplace_back(x, y, CartDir::Right);
					break;
				case '<':
					map.emplace(x, y, TrackDir::Horizontal);
					carts.emplace_back(x, y, CartDir::Left);
					break;
				case '\n':
					++y;
					x = 0;
					continue;
			}
			++x;
		}
		std::sort(carts.begin(), carts.end());
	}

	void FindCollision() {
		while (true) {
			for (auto cart = carts.begin(); cart != carts.end(); ++cart) {
				cart->Move(map);
				for (auto cart2 = carts.begin(); cart2 != carts.end(); ++cart2)
					if (cart == cart2)
						continue;
					else if (*cart == *cart2) {
						std::cout << "Crash at: " << cart->X() << ',' << cart->Y() << '\n';
						return;
					}
			}
			std::sort(carts.begin(), carts.end());
		}
	}
};

int main(int argc, const char* argv[]) {
	std::ifstream file{argc == 2 ? argv[1] : "input.txt"};
	TrackMap tm{file};
	tm.FindCollision();
}
