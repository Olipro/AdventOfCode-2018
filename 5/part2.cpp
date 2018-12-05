#include <array>
#include <future>
#include <fstream>
#include <iostream>
#include <string>

class PolymerReactor {
	std::string polymer;

	bool FindAndRemovePair(const std::string& pair) {
		bool wasChanged = false;
		for (auto i = polymer.find(pair); i != polymer.npos; i = polymer.find(pair)) {
			wasChanged = true;
			polymer.erase(i, 2);
		}
		return wasChanged;
	}

	bool DoIteration() {
		std::string pair{2, '\0'};
		bool madeChange = false;
		for (auto i = 'A'; i <= 'Z'; ++i) {
			pair[0] = i;
			pair[1] = i + 32;
			while(FindAndRemovePair(pair)) { madeChange = true; }
			pair[0] = i + 32;
			pair[1] = i;
			while(FindAndRemovePair(pair)) { madeChange = true; }
		}
		return madeChange;
	}
public:
	PolymerReactor(std::istream& in) {
		std::getline(in, polymer);
	}

	void DoFullReaction() {
		while (DoIteration() || DoIteration());
	}

	void RemoveAllUnitsOfType(char u) {
		for (auto i = polymer.find(u); i != polymer.npos; i = polymer.find(u))
			polymer.erase(i, 1);
		u += 32;
		for (auto i = polymer.find(u); i != polymer.npos; i = polymer.find(u))
			polymer.erase(i, 1);
	}

	bool operator<(const PolymerReactor& other) const {
		return polymer.size() < other.polymer.size();
	}

	size_t Size() const {
		return polymer.size();
	}
};

class ShortestPolymerFinder {
	const PolymerReactor reactor;
public:
	ShortestPolymerFinder(const PolymerReactor& reactor) : reactor{reactor} {}

	PolymerReactor FindShortestPolymer() {
		auto ret = reactor;
		std::array<std::future<PolymerReactor>, 26> computations;
		auto idx = 0;
		for (auto i = 'A'; i <= 'Z'; ++i) {
			computations[idx++] = std::async(std::launch::async, [this, i] {
				auto test = reactor;
				test.RemoveAllUnitsOfType(i);
				test.DoFullReaction();
				return test;
			});
		}
		for (auto& fut : computations) {
			auto&& test = fut.get();
			if (test < ret)
				ret = std::move(test);
		}
		return ret;
	}
};

int main(int argc, char* argv[]) {
	std::ifstream file{argc == 2 ? argv[1] : "input.txt"};
	ShortestPolymerFinder finder{PolymerReactor{file}};
	std::cout << finder.FindShortestPolymer().Size() << std::endl;
}
