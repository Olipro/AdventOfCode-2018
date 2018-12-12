#include <cstdint>
#include <iostream>
#include <utility>

class Point {
	int x;
	int y;
public:
	Point(int x, int y) noexcept : x{x}, y{y} {}
	int X() const noexcept {
		return x;
	}
	int Y() const noexcept {
		return y;
	}
};

class FuelGrid {
	Point topCorner{0, 0};
	int64_t totalPower = 0;
public:
	FuelGrid() = default;
	FuelGrid(Point topCorner, int64_t totalPower) noexcept : topCorner{std::move(topCorner)}, totalPower{totalPower} {}
	const Point& TopCorner() const noexcept {
		return topCorner;
	}
	int64_t TotalPower() const noexcept {
		return totalPower;
	}
	bool operator>(const FuelGrid& other) const noexcept {
		return totalPower > other.totalPower;
	}
};

class GridSearcher {
	Point min{1, 1};
	Point max{300, 300};
	int serialNo;

	static int IsolateHundredsDigit(int64_t num) {
		auto ret = (num / 100) % 10;
		return ret;
	}

	int64_t ComputeCellPower(const Point& loc) const noexcept {
		auto rackID = loc.X() + 10;
		auto power = loc.Y() * rackID;
		power += serialNo;
		power *= rackID;
		return IsolateHundredsDigit(power) - 5;
	}

	int64_t ComputeGridPower(const Point& topCorner, int size = 3) const noexcept {
		int64_t ret = 0;
		for (auto x = topCorner.X(); x < topCorner.X() + size; ++x)
			for (auto y = topCorner.Y(); y < topCorner.Y() + size; ++y)
				ret += ComputeCellPower({x, y});
		return ret;
	}
public:
	GridSearcher(int serialNo, const Point& min = {1,1}, const Point& max = {300, 300}) : min{min}, max{max}, serialNo{serialNo} {}

	FuelGrid FindBestFuelCells(int size = 3) const noexcept {
		FuelGrid ret;
		for (auto x = min.X(); x < max.X() - size + 1; ++x)
			for (auto y = min.Y(); y < max.Y() - size + 1; ++y) {
				const Point topCorner{x, y};
				FuelGrid next{topCorner, ComputeGridPower(topCorner, size)};
				if (next > ret)
					ret = std::move(next);
			}
		return ret;
	}
};

int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::cerr << "Please input a grid serial # as the only argument.\n";
		return 0;
	}
	GridSearcher searcher{std::stoi(argv[1])};
	auto result = searcher.FindBestFuelCells();
	std::cout << "X: " << result.TopCorner().X() << " Y: " << result.TopCorner().Y() << " Total Power: " << result.TotalPower() << '\n';
}
