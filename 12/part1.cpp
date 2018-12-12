#include <fstream>
#include <iostream>
#include <set>
#include <utility>
#include <vector>

class PotTransformation {
	bool left2;
	bool left1;
	bool self;
	bool right1;
	bool right2;

	bool newState;

public:
	PotTransformation(std::istream& in) {
		left2 = in.get() == '#';
		left1 = in.get() == '#';
		self = in.get() == '#';
		right1 = in.get() == '#';
		right2 = in.get() == '#';
		in.seekg(4, std::ios_base::cur);
		newState = in.get() == '#';
		in.seekg(1, std::ios_base::cur);
	}
	bool Left2() const noexcept {
		return left2;
	}
	bool Left1() const noexcept {
		return left1;
	}
	bool Self() const noexcept {
		return self;
	}
	bool Right1() const noexcept {
		return right1;
	}
	bool Right2() const noexcept {
		return right2;
	}
	bool NewState() const noexcept {
		return newState;
	}
};

class Greenhouse {
	std::set<int> plants;
	std::vector<PotTransformation> xfrms;

	static decltype(plants) SetupPots(std::istream& in) {
		int id = 0;
		decltype(plants) ret;
		in.seekg(15, std::ios_base::cur);
		std::string initial;
		std::getline(in, initial);
		for (auto i = initial.begin(); i != initial.end(); ++i)
			if (*i == '#')
				ret.emplace(id++);
			else
				++id;
		in.seekg(1, std::ios_base::cur);
		return ret;
	}

	static decltype(xfrms) SetupXfrms(std::istream& in) {
		decltype(xfrms) ret;
		while (in)
			ret.emplace_back(in);
		ret.pop_back();
		return ret;
	}

	bool IDOccupied(const int id) const noexcept {
		return plants.find(id) != plants.end();
	}

	bool IsMatch(const int id, const PotTransformation& xfrm) const noexcept {
		return	xfrm.Left2() == IDOccupied(id - 2) &&
			xfrm.Left1() == IDOccupied(id - 1) &&
			xfrm.Self()  == IDOccupied(id) &&
			xfrm.Right1() == IDOccupied(id + 1) &&
			xfrm.Right2() == IDOccupied(id + 2);
	}
public:
	Greenhouse(std::istream& in) : plants{SetupPots(in)}, xfrms{SetupXfrms(in)} {}

	void DoGeneration() {
		std::set<int> nextGen;
		for (const auto& plant : plants) {
			for (auto id = plant - 2; id <= plant + 2; ++id) {
				for (const auto& xfrm : xfrms) {
					if (xfrm.NewState() && IsMatch(id, xfrm))
						nextGen.emplace(id);
				}
			}
		}
		plants = std::move(nextGen);
	}

	int SumPlants() const noexcept {
		int ret = 0;
		for (const auto& id : plants)
			ret += id;
		return ret;
	}
};

int main(int argc, char* argv[]) {
	std::ifstream file{argc == 2 ? argv[1] : "input.txt"};
	Greenhouse gh{file};
	for (auto i = 1; i < 21; ++i)
		gh.DoGeneration();
	std::cout << "Sum: " << gh.SumPlants() << '\n';
}
