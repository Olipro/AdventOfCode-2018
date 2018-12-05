#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using Value = int;
using Occurrences = int;

int main(int argc, char* argv[]) {
	std::ifstream in{argc == 2 ? argv[1] : "input.txt"};
	std::string line;
	std::unordered_map<Value, Occurrences> values;
	std::vector<int> data;
	int val = 0;
	while (std::getline(in, line))
		data.emplace_back(std::stoi(line));
	while (true) {
		for (const auto& i : data) {
			val += i;
			if (++values[val] > 1) {
				std::cout << val << std::endl;
				return 0;
			}
		}
	}
}
