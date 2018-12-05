#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
	std::ifstream in{argc == 2 ? argv[1] : "input.txt"};
	std::string line;
	int val = 0;
	while (std::getline(in, line))
		val += std::stoi(line);
	std::cout << val << std::endl;
}
