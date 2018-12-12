#include <cmath>
#include <cstdint>
#include <iostream>
#include <list>
#include <string>
#include <vector>

class Marble {
	uint32_t id;
public:
	Marble(uint32_t id) noexcept : id{id} {}

	uint32_t ID() const noexcept {
		return id;
	}
};

class MarbleCircle {
	class CircleIterator {
		std::list<Marble>& marbles;
		std::list<Marble>::iterator iter;
		int index;

		int ComputeIndex(int idx) const noexcept {
			int size = marbles.size();
			while (idx < 0)
				idx += size;
			return idx % size;
		}
	public:
		CircleIterator(std::list<Marble>& marbles) noexcept : marbles{marbles}, iter{marbles.begin()}, index{0} {}
		CircleIterator(std::list<Marble>& marbles, int idx) noexcept : marbles{marbles}, iter{marbles.begin()}, index{ComputeIndex(idx)} {
			auto size = marbles.size();
			if (idx - (size / 2) > 0) {
				iter = marbles.end();
				while (size-- > index)
				       --iter;
			} else
				while (size++ < index)
					++iter;
		}
		CircleIterator(const CircleIterator& other) noexcept : marbles{other.marbles}, iter{other.iter}, index{other.index} {}
		CircleIterator(CircleIterator&& other) noexcept : marbles{other.marbles}, iter{std::move(other.iter)}, index{other.index} {}
		CircleIterator& operator++() noexcept {
			++index;
			++iter;
			if (iter == marbles.end()) {
				iter = marbles.begin();
				index = 0;
			}
			return *this;
		}
		CircleIterator& operator--() noexcept {
			if (--index < 0) {
				index = marbles.size() - 1;
				iter = --marbles.end();
			} else
				--iter;
			return *this;
		}
		CircleIterator operator+(int offset) const noexcept {
			CircleIterator ret = *this;
			ret.index = ComputeIndex(index + offset);
			if (index < ret.index) {
				auto tgt = ret.index - index;
				while (tgt--)
					++ret.iter;
			} else if (index > ret.index) {
				auto tgt = index - ret.index;
				while (tgt--)
					--ret.iter;
			}
			return ret;
		}
		CircleIterator operator-(int offset) const noexcept {
			return operator+(-offset);
		}
		CircleIterator& operator=(int newIndex) noexcept {
			auto oldIndex = index;
			index = ComputeIndex(newIndex);
			if (index == 0)
				iter = marbles.begin();
			else if (index == (marbles.size() - 1))
				iter = --marbles.end();
			else {
				if (oldIndex < index)
					while (oldIndex++ < index)
						++iter;
				else if (oldIndex > index)
					while (oldIndex-- > index)
						--iter;
			}
			return *this;
		}
		CircleIterator& operator=(const CircleIterator& rhs) noexcept {
			index = rhs.index;
			iter = rhs.iter;
			return *this;
		}
		CircleIterator& operator=(CircleIterator&& rhs) noexcept {
			index = std::move(rhs.index);
			iter = std::move(rhs.iter);
			return *this;
		}
		Marble& operator*() noexcept {
			return *iter;
		}
		const Marble& operator*() const noexcept {
			return *iter;
		}
		Marble* operator->() noexcept {
			return &*iter;
		}
		const Marble* operator->() const noexcept {
			return &*iter;
		}
		const std::list<Marble>::iterator Iter() const noexcept {
			return iter;
		}
		std::list<Marble>::iterator Iter() noexcept {
			return iter;
		}
		size_t Index() const noexcept {
			return index;
		}
		void NotifyDeleted(const CircleIterator& del) {
			if (index > del.index)
				--index;
		}
	};
	std::list<Marble> marbles{{0}};
	CircleIterator currentMarble{marbles};
	uint32_t playId = 0;

	uint32_t DoNormalPlace(const int newId) {
		switch (marbles.size()) {
			default:
				auto pos = currentMarble + 2;
				auto sav = pos - 1;
				auto idx = pos.Index();
				if (idx) {
					marbles.emplace(pos.Iter(), newId);
					currentMarble = ++sav;
				} else {
					marbles.emplace_back(newId);
					currentMarble = Last();
				}
			break;
		}
		return 0;
	}

	uint32_t DoSpecialPlace(const int newId) {
		auto pos = currentMarble - 7;
		auto sav = pos + 1;
		auto ret = pos->ID() + newId;
		marbles.erase(pos.Iter());
		sav.NotifyDeleted(pos);
		currentMarble = std::move(sav);
		return ret;
	}
public:
	CircleIterator CurrentMarble() const noexcept {
		return currentMarble;
	}

	CircleIterator First() noexcept {
		return {marbles, 0};
	}

	CircleIterator Last() noexcept {
		return {marbles, static_cast<int>(marbles.size()) - 1};
	}

	uint32_t AddMarble() {
		auto newId = ++playId;
		if (newId % 23)
			return DoNormalPlace(newId);
		else
			return DoSpecialPlace(newId);
	}

	uint32_t LastID() const noexcept {
		return playId;
	}
};

class Player {
	uint32_t score = 0;
public:
	void AddPoints(uint32_t score) noexcept {
		this->score += score;
	}

	uint32_t Score() const noexcept {
		return score;
	}
};

class GameBoard {
	MarbleCircle marbleCircle;
public:
	const Marble& CurrentMarble() const noexcept {
		return *marbleCircle.CurrentMarble();
	}

	uint32_t SimulatePlay(int lastMarbleValue, size_t playerCount) {
		std::vector<Player> players{playerCount};
		while (true) {
			for (auto& player: players) {
				player.AddPoints(marbleCircle.AddMarble());
				if (marbleCircle.LastID() != lastMarbleValue)
					continue;
				uint32_t ret = 0;
				for (auto& player : players)
					ret = std::max(ret, player.Score());
				return ret;
			}
		}
	}
};

int main(int argc, char* argv[]) {
	if (argc != 3) {
		std::cerr << "Wrong number of arguments, expect [playerCount] [lastMarbleValue]\n";
		return 0;
	}
	size_t playerCount = std::stoi(argv[1]);
	size_t lastMarbleValue = std::stoi(argv[2]);
	GameBoard game;
	std::cout << playerCount <<" Players, " << "Last Marble: " << lastMarbleValue << std::endl;
	std::cout << "High Score: " << game.SimulatePlay(lastMarbleValue, playerCount) << std::endl;
}
