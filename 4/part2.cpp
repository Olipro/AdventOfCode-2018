#include <chrono>
#include <ctime>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <vector>

using Seconds = std::chrono::seconds;
using Minute = uint32_t;
using Occurrences = uint32_t;
using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

using namespace std::chrono_literals;
using namespace std::string_literals;

template<typename T>
uint8_t FindHighestIndex(const std::vector<T>& occurrences) {
	for (uint8_t i = 0; i < 60; ++i) {
		bool isGreater = true;
		for (auto& j : occurrences) {
			if (occurrences[i] < j) {
				isGreater = false;
				break;
			}
		}
		if (isGreater)
			return i;
	}
	return 0; //should be unreachable
}

enum class ActionType {
	start_shift,
	fall_asleep,
	awaken
};

struct DataEntry {
	TimePoint time;
	ActionType action;
	uint16_t guardID = 0;
	DataEntry(TimePoint time, ActionType action) : time{time}, action{action} {}
	DataEntry(TimePoint time, ActionType action, uint16_t guardID) : time{time}, action{action}, guardID{guardID} {}
	bool operator<(const DataEntry& other) const {
		return time < other.time;
	}
};

std::ostream& operator<<(std::ostream& os, const DataEntry& d) {
	auto t = std::chrono::system_clock::to_time_t(d.time);
	os << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M ");
	switch (d.action) {
		case ActionType::start_shift:
			os << "- Start Shift" << " - ID: " << d.guardID;
		break;
		case ActionType::fall_asleep:
			os << "- Fall Asleep";
		break;
		case ActionType::awaken:
			os << "- Awaken";
		break;
	}
	return os;
}

class SortedDataEntries {
	std::vector<DataEntry> entries;
public:
	SortedDataEntries(std::istream& in) {
		while (in) {
			std::tm tm{};
			in >> std::get_time(&tm, "[%Y-%m-%d %H:%M] ");
			TimePoint t = std::chrono::system_clock::from_time_t(std::mktime(&tm));
			std::string action;
			std::getline(in, action);
			if (action[0] == 'f') {
				entries.emplace_back(t, ActionType::fall_asleep);
			} else if(action[0] == 'w') {
				entries.emplace_back(t, ActionType::awaken);
			} else {
				std::istringstream s{std::move(action)};
				s.seekg(7, std::ios_base::cur);
				uint32_t id;
				s >> id;
				entries.emplace_back(t, ActionType::start_shift, id);
			}
		}
		std::sort(entries.begin(), entries.end());
	}
	const std::vector<DataEntry>& GetEntries() const {
		return entries;
	}
};

class SleepPeriod {
	TimePoint start;
	TimePoint end;
public:
	SleepPeriod(TimePoint start, TimePoint end) : start{start}, end{end} {}
	TimePoint Start() const {
		return start;
	}
	TimePoint End() const {
		return end;
	}
	Seconds Duration() const {
		return std::chrono::duration_cast<Seconds>(end - start);
	}
	bool operator>(const SleepPeriod& other) const {
		return Duration() > other.Duration();
	}
	bool operator<(const SleepPeriod& other) const {
		return Duration() < other.Duration();
	}
};

class Guard {
	TimePoint shiftStart;
	std::vector<SleepPeriod> sleepTimes;
	uint32_t id;
public:
	Guard(TimePoint shiftStart, uint32_t id) : shiftStart{shiftStart}, id{id} {}
	void AddSleepPeriod(SleepPeriod activity) {
		sleepTimes.emplace_back(std::move(activity));
	}
	bool HasLongerSleepTime(const Guard& other) const {
		auto ourDur = 0s;
		auto theirDur = 0s;
		for (const auto& sleep : sleepTimes)
			ourDur += sleep.Duration();
		for (const auto& sleep : other.sleepTimes)
			theirDur += sleep.Duration();
		return ourDur > theirDur;
	}
	const std::vector<SleepPeriod>& GetSleepTimes() const {
		return sleepTimes;
	}
	SleepPeriod GetLongestSleepTime() const {
		SleepPeriod ret{std::chrono::system_clock::now(), std::chrono::system_clock::now() + 1s};
		for (const auto& time : sleepTimes)
			if (time > ret)
				ret = time;
		return ret;
	}
	std::pair<Minute, Occurrences> GetMinuteMostSpentSleepingInfo() const {
		auto longestTime = GetLongestSleepTime();
		std::vector<uint8_t> occurrences(60);
		for (auto& sleep : sleepTimes) {
			auto cur = sleep.Start();
			auto end = sleep.End();
			for (cur = sleep.Start(); cur < sleep.End(); cur += 1min) {
				auto tm = std::chrono::system_clock::to_time_t(cur);
				auto t = std::localtime(&tm);
				++occurrences[t->tm_min];
			}
		}
		auto idx = FindHighestIndex(occurrences);
		return {idx, occurrences[idx]};
	}
	uint32_t ID() const {
		return id;
	}
};

std::ostream& operator<<(std::ostream& os, const Guard& guard) {
	auto longestSleep = guard.GetLongestSleepTime();
	auto start = std::chrono::system_clock::to_time_t(longestSleep.Start());
	auto end = std::chrono::system_clock::to_time_t(longestSleep.End());
	os << "ID: " << std::to_string(guard.ID())
	   << " Longest Sleep Start: " << std::put_time(std::localtime(&start), "%Y-%m-%d %H:%M")
	   << " Longest Sleep End: " << std::put_time(std::localtime(&end), "%Y-%m-%d %H:%M") << std::endl;
	for (const auto& sleep : guard.GetSleepTimes()) {
		auto start = std::chrono::system_clock::to_time_t(sleep.Start());
		auto end = std::chrono::system_clock::to_time_t(sleep.End());
		os << " Sleep Start: " << std::put_time(std::localtime(&start), "%Y-%m-%d %H:%M")
		   << " Sleep End: " << std::put_time(std::localtime(&end), "%Y-%m-%d %H:%M") << std::endl;
	}
	return os;
}

class GuardShifts {
	std::unordered_map<uint32_t, Guard> guards;
public:
	GuardShifts(const SortedDataEntries& entries) {
		const auto& vec = entries.GetEntries();
		for (auto entry = vec.cbegin(); entry != vec.cend();) {
			if (entry->action != ActionType::start_shift)
				throw std::logic_error{"ABORTING, WRONG ENTRY TYPE"};
			auto& guard = guards.try_emplace(entry->guardID, entry->time, entry->guardID).first->second;
			while (++entry != vec.cend() && entry->action != ActionType::start_shift) {
				auto sleepStart = entry->time;
				auto sleepEnd = (++entry)->time;
				guard.AddSleepPeriod({sleepStart, sleepEnd});
			}
		}
	}
	const Guard& FindLaziest() const {
		static const Guard dummyGuard{std::chrono::system_clock::now(), 0};
		std::reference_wrapper<const Guard> ret = dummyGuard;
		for (const auto& pair : guards) {
			const Guard& guard = pair.second;
			if (!ret.get().HasLongerSleepTime(guard))
				ret = guard;
		}
		return ret.get();
	}
	const Guard& FindGuardWithMostCommonSleepMinute() const {
		static const Guard dummyGuard{std::chrono::system_clock::now(), 0};
		std::reference_wrapper<const Guard> ret = dummyGuard;
		for (const auto& pair : guards) {
			const Guard& guard = pair.second;
			auto info = guard.GetMinuteMostSpentSleepingInfo();
			if (info.second > ret.get().GetMinuteMostSpentSleepingInfo().second)
				ret = guard;
		}
		return ret.get();
	}
};

int main(int argc, char* argv[]) {
	std::ifstream file{argc == 2 ? argv[1] : "input.txt"};
	SortedDataEntries entries{file};
	GuardShifts shifts{entries};
	auto guard = shifts.FindGuardWithMostCommonSleepMinute();
	auto info = guard.GetMinuteMostSpentSleepingInfo();
	auto id = guard.ID();
	std::cout << "ID: " << id << " Most Common Minute: " << info.first << " Occurrences: " << info.second << " Answer: " << info.first * id << std::endl;
}
