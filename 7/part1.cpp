#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>
#include <unordered_set>
#include <utility>
#include <vector>

class Step {
public:
	Step(char id) noexcept : id{id} {}
	
	const Step& AddParent(const Step& step) const {
		parents.emplace(step);
		return *this;
	}

	const Step& RemoveParent(const Step& step) const {
		parents.erase(step);
		return *this;
	}

	char ID() const noexcept {
		return id;
	}

	bool operator==(const Step& other) const noexcept {
		return id == other.id;
	}

	bool IsMyParent(const Step& other) const noexcept {
		if (other == *this)
			return false;
		if (parents.find(other) != parents.end())
			return true;
		for (const auto& dep : parents)
			if (dep->IsMyParent(other))
				return true;
		return false;
	}

	bool operator<(const Step& other) const noexcept {
		return id < other.id;
	}

	struct StepHash {
		size_t operator()(const Step& step) const noexcept {
			return step.id;
		}
	};

	struct StepRef {
		const Step* step;
	public:
		StepRef(const Step& step) noexcept : step{std::addressof(step)} {}

		operator const Step&() const noexcept {
			return *step;
		}

		bool operator==(const StepRef& other) const noexcept {
			return (*step) == *other.step;
		}

		bool operator<(const StepRef& other) const noexcept {
			return (*step) < *other.step;
		}

		const Step* operator->() const noexcept {
			return step;
		}
	};
private:
	char id;
	mutable std::unordered_set<StepRef, StepHash> parents;
};

class StepManager {
	const std::unordered_set<Step, Step::StepHash> stepsUnique;
	std::vector<Step::StepRef> sortedSteps;

	static auto GetNextAvailableStep(const std::unordered_set<Step::StepRef, Step::StepHash>& scratch) {
		std::vector<Step::StepRef> candidates;
		for (const auto& step : scratch) {
			bool available = true;
			for (const auto& cmp : scratch) {
				if (!step->IsMyParent(cmp))
					continue;
				available = false;
				break;
			}
			if (available)
				candidates.emplace_back(step);
		}
		if (candidates.size() > 1)
			std::partial_sort(candidates.begin(), candidates.begin() + 1, candidates.end());
		return candidates;
	}

	static auto SortSteps(const decltype(stepsUnique)& stepsUnique) {
		std::unordered_set<Step::StepRef, Step::StepHash> scratch{stepsUnique.begin(), stepsUnique.end()};
		decltype(sortedSteps) ret;
		while (scratch.size()) {
			auto step = GetNextAvailableStep(scratch);
			if (step.empty())
				break;
			ret.emplace_back(std::move(step.front()));
			scratch.erase(ret.back());
			for (auto&& step : scratch)
				step->RemoveParent(ret.back());
		}
		return ret;
	}

	static auto InitializeSteps(std::istream& in) {
		std::unordered_set<Step, Step::StepHash> stepsUnique;
		while (in) {
			in.seekg(5, std::ios_base::cur);
			char id = in.get();
			if (!in)
				break;
			auto& step = *stepsUnique.emplace(id).first;
			in.seekg(30, std::ios_base::cur);
			id = in.get();
			auto& child = *stepsUnique.emplace(id).first;
			child.AddParent(step);
			in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
		return stepsUnique;
	}
public:
	StepManager(std::istream& in) : stepsUnique{InitializeSteps(in)}, sortedSteps{SortSteps(stepsUnique)} {}
	const auto& Steps() const noexcept {
		return sortedSteps;
	}
};

int main(int argc, char* argv[]) {
	std::ifstream file{argc == 2 ? argv[1] : "input.txt"};
	StepManager stepManager{file};
	for (const auto& step : stepManager.Steps())
		std::cout << step->ID();
	std::cout << '\n';
}

