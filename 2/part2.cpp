#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

void FindMatchingIDs(const std::vector<std::string>& data) {
	for (const auto& first : data) {
		for (const auto& second : data) {
			if (first.size() != second.size())
				continue;
			auto diffCnt = 0;
			auto pos = 0;
			for (auto i = 0; i < first.size(); ++i) {
				if (first[i] != second[i]) {
					++diffCnt;
					pos = i;
				}
				if (diffCnt > 1) break;
			}
			if (diffCnt == 1) {
				std::cout << std::string{first}.erase(pos, 1) << std::endl;
				return;
			}
		}
	}
}

int main(int argc, char* argv[]) {
	std::ifstream in{argc == 2 ? argv[1] : "input.txt"};
	std::vector<std::string> data;
	std::string line;
	line.reserve(1000);
	while (std::getline(in, line))
		data.emplace_back(std::move(line));
	FindMatchingIDs(data);
}
