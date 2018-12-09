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

	OptionalPoint FindNearestNeighbour(const Coordinate& c) noexcept {
		auto lastDistance = points.front().Distance(c);
		OptionalPoint ret;
		for (auto& point : points) {
			auto distance = point.Distance(c);
			if (distance == 0)
				return std::ref(point);
			if (distance < lastDistance) {
				lastDistance = distance;
				ret = std::ref(point);
			}
		}
		auto distCnt = 0;
		for (const auto& point : points)
			if (point.Distance(c) == lastDistance && ++distCnt == 2)
				return {};
		return ret;
	}

	void ComputeNearestNeighbours() noexcept {
		for (auto x = min.X(); x <= max.X(); ++x)
			for (auto y = min.Y(); y <= max.Y(); ++y) {
				auto nn = FindNearestNeighbour({x, y});
				if (nn)
					++nn->get();
			}
	}

	void RemoveInfinitePoints() noexcept {
		auto i = std::remove_if(points.begin(), points.end(), [this] (const auto& point) {
			for (auto x = min.X(); x <= max.X(); ++x) {
				auto nn = FindNearestNeighbour({x, min.Y()});
				if (nn && point == *nn)
					return true;
				nn = FindNearestNeighbour({x, max.Y()});
				if (nn && point == *nn)
					return true;
			}
			for (auto y = min.Y(); y <= max.Y(); ++y) {
				auto nn = FindNearestNeighbour({min.X(), y});
				if (nn && point == *nn)
					return true;
				nn = FindNearestNeighbour({max.X(), y});
				if (nn && point == *nn)
					return true;
			}
			return false;
		});
		points.erase(i, points.end());
	}
public:
	PointMap(std::istream& in) : points{PopulatePoints(in)}, min{GetMinPoint(points)}, max{GetMaxPoint(points)} {
		ComputeNearestNeighbours();
		RemoveInfinitePoints();
	}

	const Point& GetPointWithMostNeighbours() const noexcept {
		auto ret = std::ref(points.front());
		for (const auto& point : points)
			if (point.NeighbourCount() > ret.get().NeighbourCount())
				ret = point;
		return ret;
	}
};

int main(int argc, char* argv[]) {
	std::ifstream file{argc == 2 ? argv[1] : "input.txt"};
	PointMap pointMap{file};
	const auto& p = pointMap.GetPointWithMostNeighbours();
	std::cout << "X: " << p.X() << " Y: " << p.Y() << " Neighbours: " << p.NeighbourCount() << '\n';
}
