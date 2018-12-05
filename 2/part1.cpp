#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

struct Counts {
	uint32_t twice = 0;
	uint32_t thrice = 0;
};

void UpdateOccurrences(const std::string& s, Counts& counts) {
	std::unordered_map<char, uint8_t> chk;
	uint8_t twice = 0, thrice = 0;
	for (auto& c : s)
		++chk[c];
	for (auto& p : chk) {
		if (p.second == 3)
			thrice = 1;
		else if (p.second == 2)
			twice = 1;
		if (twice && thrice) break;
	}
	counts.twice += twice;
	counts.thrice += thrice;
}

void CalculateChecksum(const std::vector<std::string>& data) {
	Counts total;
	for (const auto& i : data) {
		UpdateOccurrences(i, total);
	}
	std::cout << total.twice * total.thrice << std::endl;
}

int main(int argc, char* argv[]) {
	std::ifstream in{argc == 2 ? argv[1] : "input.txt"};
	std::vector<std::string> data;
	std::string line;
	while (std::getline(in, line))
		data.emplace_back(std::move(line));
	CalculateChecksum(data);
}
