#include <cstdint>
#include <fstream>
#include <iostream>
#include <utility>
#include <vector>

class Metadata {
	int value;
public:
	Metadata(std::istream& in) {
		in >> value;
	}

	int Value() const noexcept {
		return value;
	}
};

class Node {
	std::vector<Node> childNodes;
	std::vector<Metadata> metadataEntries;
public:
	Node(std::istream& in) {
		int childNodeCount, metadataCount;
		in >> childNodeCount;
		in >> metadataCount;
		childNodes.reserve(childNodeCount);
		metadataEntries.reserve(metadataCount);
		while (childNodeCount--)
			childNodes.emplace_back(in);
		while (metadataCount--)
			metadataEntries.emplace_back(in);
	}

	int MetadataSum() const noexcept {
		int ret = 0;
		for (const auto& metadata : metadataEntries)
			ret += metadata.Value();
		for (const auto& child : childNodes)
			ret += child.MetadataSum();
		return ret;
	}

	int Value() const noexcept {
		int ret = 0;
		if (childNodes.size() == 0)
			for (const auto& metadata : metadataEntries)
				ret += metadata.Value();
		else
			for (const auto& metadata : metadataEntries) {
				auto val = metadata.Value();
				if (val == 0 || val > childNodes.size())
					continue;
				ret += childNodes[val - 1].Value();
			}
		return ret;
	}
};

int main(int argc, char* argv[]) {
	std::ifstream file{argc == 2 ? argv[1] : "input.txt"};
	Node rootNode{file};
	std::cout << rootNode.Value() << '\n';
}
