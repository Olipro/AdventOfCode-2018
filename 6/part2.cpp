#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <optional>
#include <vector>

class Coordinate {
	int x;
	int y;
public:
	Coordinate(int x, int y) noexcept : x{x}, y{y} {}
	int X() const noexcept {
		return x;
	}
	int Y() const noexcept {
		return y;
	}
	bool operator==(const Coordinate& other) const noexcept {
		return x == other.x && y == other.y;
	}
	int Distance(const Coordinate& other) const noexcept {
		return std::abs(x - other.x) + std::abs(y - other.y);
	}
};

class Point : public Coordinate {
	uint32_t neighbourCount = 0;
public:
	using Coordinate::Coordinate;

	Point& operator++() noexcept {
		++neighbourCount;
		return *this;
	}

	uint32_t NeighbourCount() const noexcept {
		return neighbourCount;
	}
};

class PointMap {
	using OptionalPoint = std::optional<std::reference_wrapper<Point>>;

	std::vector<Point> points;
	const Point min;
	const Point max;

	static decltype(points) PopulatePoints(std::istream& in) {
		decltype(points) ret;
		int x, y;
		while (in) {
			in >> x;
			in.seekg(2, std::ios_base::cur);
			in >> y;
			in.ignore(1, '\n');
			if (!in)
				break;
			ret.emplace_back(x, y);
		}
		return ret;
	}

	static Point GetMinPoint(const decltype(points)& points) noexcept {
		auto x = points.front().X();
		auto y = points.front().Y();
		for (const auto& point : points) {
			auto tmpX = point.X();
			auto tmpY = point.Y();
			if (tmpX < x)
				x = tmpX;
			if (tmpY < y)
				y = tmpY;
		}
		return {x - 1, y - 1};
	}

	static Point GetMaxPoint(const decltype(points)& points) noexcept {
		auto x = points.front().X();
		auto y = points.front().Y();
		for (const auto& point : points) {
			auto tmpX = point.X();
			auto tmpY = point.Y();
			if (tmpX > x)
				x = tmpX;
			if (tmpY > y)
				y = tmpY;
		}
		return {x + 1, y + 1};
	}

public:
	PointMap(std::istream& in) : points{PopulatePoints(in)}, min{GetMinPoint(points)}, max{GetMaxPoint(points)} {
	}

	uint32_t FindRegionWithinSize(uint32_t limit) {
		uint32_t ret = 0;
		for (auto x = min.X(); x <= max.X(); ++x)
			for (auto y = min.Y(); y <= max.Y(); ++y) {
				const Point p{x, y};
				auto pointDist = 0;
				for (const auto& point : points) {
					pointDist += point.Distance(p);
					if (pointDist > limit)
						break;
				}
				if (pointDist < limit)
					++ret;
			}
		return ret;
	}
};

int main(int argc, char* argv[]) {
	std::ifstream file{argc >= 2 ? argv[1] : "input.txt"};
	PointMap pointMap{file};
	std::cout << pointMap.FindRegionWithinSize(argc == 3 ? std::stoi(argv[2]) : 10000) << '\n';
}
