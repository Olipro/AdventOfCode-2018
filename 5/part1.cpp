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

	const std::string& GetPolymer() const {
		return polymer;
	}
};

int main(int argc, char* argv[]) {
	std::ifstream file{argc == 2 ? argv[1] : "input.txt"};
	PolymerReactor reactor{file};
	reactor.DoFullReaction();
	std::cout << reactor.GetPolymer().size() << std::endl;
}
