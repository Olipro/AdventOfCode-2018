#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <tuple>
#include <utility>
#include <vector>

class Point {
protected:
	int x;
	int y;
public:
	Point(int x, int y) : x{x}, y{y} {}
	
	int X() const noexcept {
		return x;
	}
	int Y() const noexcept {
		return y;
	}
	
	bool operator==(const Point& other) const noexcept {
		return x == other.x && y == other.y;
	}
};

class MovingPoint : public Point {
	const int xChg;
	const int yChg;
public:
	MovingPoint(int x, int y, int xChg, int yChg) noexcept : Point{x, y}, xChg{xChg}, yChg{yChg} {}

	void Tick() {
		x += xChg;
		y += yChg;
	}

	void UndoTick() {
		x -= xChg;
		y -= yChg;
	}
};

struct PointLimit {
	Point min;
	Point max;
};

class PointManager {
	std::vector<MovingPoint> points;
	unsigned tickCount = 0;

	static std::vector<MovingPoint> InitializePoints(std::istream& in) {
		std::vector<MovingPoint> ret;
		int x, y, xChg, yChg;
		while (in) {
			in.seekg(10, std::ios_base::cur);
			in >> x;
			if (!in)
				break;
			in.seekg(1, std::ios_base::cur);
			in >> y;
			in.seekg(12, std::ios_base::cur);
			in >> xChg;
			in.seekg(1, std::ios_base::cur);
			in >> yChg;
			in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			ret.emplace_back(x, y, xChg, yChg);
		}
		return ret;
	}

	PointLimit MinMax() const {
		auto xMin = std::numeric_limits<int>::max(), yMin = xMin;
		auto xMax = std::numeric_limits<int>::min(), yMax = xMax;
		for (const auto& point : points) {
			if (point.X() < xMin)
				xMin = point.X();
			if (point.Y() < yMin)
				yMin = point.Y();
			if (point.X() > xMax)
				xMax = point.X();
			if (point.Y() > yMax)
				yMax = point.Y();
		}
		return {Point{xMin, yMin}, Point{xMax, yMax}};
	}
	
	void Tick() noexcept {
		++tickCount;
		for (auto& point : points)
			point.Tick();
	}
	
	void UndoTick() noexcept {
		--tickCount;
		for (auto& point : points)
			point.UndoTick();
	}
public:
	PointManager(std::istream& in) : points{InitializePoints(in)} {}

	void FindAndPrintText() {
		auto limit = MinMax();
		for (; (std::abs(limit.max.Y()) - std::abs(limit.min.Y())) > 9; Tick())
			limit = MinMax();
		UndoTick();
		for (auto y = limit.min.Y(); y <= limit.max.Y(); ++y) {
			for (auto x = limit.min.X(); x <= limit.max.X(); ++x) {
				if (std::find(points.begin(), points.end(), Point{x, y}) != points.end())
					std::cout << '#';
				else
					std::cout << '.';
			}
			std::cout << '\n';
		}
		std::cout << "\nTick Count: " << tickCount << std::endl;
	}
	
};

int main(int argc, char* argv[]) {
	std::ifstream file{argc == 2 ? argv[1] : "input.txt"};
	PointManager p{file};
	p.FindAndPrintText();
}

