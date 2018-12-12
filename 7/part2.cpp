#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <set>
#include <unordered_set>
#include <utility>
#include <vector>

//Main Challenge settings
constexpr auto timeOffset = 4;
constexpr auto workerCount = 5;
//Test settings
//constexpr auto timeOffset = 64;
//constexpr auto workerCount = 2;

class Step {
public:
	Step(char id) noexcept : id{id}, timeRemaining{static_cast<int>(id) - timeOffset} {}
	
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

	uint32_t TimeRemaining() const noexcept {
		return timeRemaining;
	}

	void DoTick() const noexcept {
		--timeRemaining;
	}

	bool operator==(const Step& other) const noexcept {
		return id == other.id;
	}

	bool HasParents() const noexcept {
		return parents.size() > 0;
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

		const Step* operator->() const noexcept {
			return step;
		}
	};
private:
	char id;
	mutable int timeRemaining;
	mutable std::unordered_set<StepRef, StepHash> parents;
};

class StepManager {
	const std::unordered_set<Step, Step::StepHash> stepsUnique;
	int workTotal;

	static auto GetNextAvailableSteps(const std::unordered_set<Step::StepRef, Step::StepHash>& scratch, int limit) {
		std::vector<Step::StepRef> candidates;
		for (const auto& step : scratch) {
			if (!step->HasParents() && limit--)
				candidates.emplace_back(step);
			if (!limit)
				break;
		}
		return candidates;
	}

	static auto CalculateWork(const decltype(stepsUnique)& stepsUnique) {
		std::unordered_set<Step::StepRef, Step::StepHash> scratch{stepsUnique.begin(), stepsUnique.end()};
		auto elapsed = 0;
		while (scratch.size()) {
			auto steps = GetNextAvailableSteps(scratch, workerCount);
			for (auto&& step : steps) {
				step->DoTick();
				if (step->TimeRemaining() == 0) {
					for (auto& i : scratch)
						i->RemoveParent(step);
					scratch.erase(step);
				}
			}
			++elapsed;
		}
		return elapsed;
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
	StepManager(std::istream& in) : stepsUnique{InitializeSteps(in)}, workTotal{CalculateWork(stepsUnique)} {}

	uint32_t WorkTotal() const noexcept {
		return workTotal;
	}
};

int main(int argc, char* argv[]) {
	std::ifstream file{argc == 2 ? argv[1] : "input.txt"};
	StepManager stepManager{file};
	std::cout << stepManager.WorkTotal() << '\n';
}

