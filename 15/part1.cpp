#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <unordered_set>
#include <utility>
#include <vector>

class Tile {
	uint16_t x;
	uint16_t y;
public:
	Tile(const uint16_t x, const uint16_t y) : x{x}, y{y} {}

	uint16_t Distance(const Tile& other) const noexcept {
        return std::abs(x - other.x) + std::abs(y - other.y);
    }

	bool operator==(const Tile& other) const noexcept {
		return x == other.x && y == other.y;
	}

	bool operator!=(const Tile& other) const noexcept {
	    return !(*this == other);
	}

	bool operator<(const Tile& other) const noexcept {
	    return y < other.y || (y == other.y && x < other.x);
	}

	uint16_t X() const noexcept {
        return x;
    }

    uint16_t Y() const noexcept {
        return y;
    }

    struct Hash {
	    size_t operator()(const Tile& n) const noexcept {
	        return n.X() << 16 | n.Y();
	    }
	};
};

class Node : public Tile {
    mutable std::vector<std::reference_wrapper<const Node>> neighbours;
    mutable std::optional<std::reference_wrapper<const Node>> parent;
public:
    Node(const Tile& tile) noexcept : Tile{tile} {}
    Node(const Node& node) : Tile{node}, neighbours{node.neighbours} {}
    Node(Node&& other) noexcept : Tile{std::move(other)}, neighbours{std::move(other.neighbours)} {}

    Node& operator=(Node&& rhs) noexcept {
        neighbours = std::move(rhs.neighbours);
        static_cast<Tile&>(*this) = std::move(rhs);
        return *this;
    }

    void AddNeighbour(const Node& tile) const {
        neighbours.emplace_back(tile);
    }

    void SetParent(const Node& node) const {
        parent = node;
    }

    const decltype(neighbours)& Neighbours() const noexcept {
        return neighbours;
    }

    decltype(parent) GetParent() const noexcept {
        return parent;
    }
};

enum class UnitType {
    Elf,
    Goblin,
    Wall,
    Empty
};

class Unit {
public:
    virtual int Health() const = 0;
    virtual int Power() const = 0;
    virtual UnitType Type() const noexcept = 0;
    virtual ~Unit() = default;
};

class Graph {
    std::unordered_set<Node, Node::Hash> graph;

    static void AddNeighbours(const decltype(graph)& graph, const Node& node) {
        auto x = node.X();
        auto y = node.Y();
        if (auto n = graph.find(Tile{x, --y}); n != graph.end())
            node.AddNeighbour(*n);
        ++y;
        if (auto n = graph.find(Tile{--x, y}); n != graph.end())
            node.AddNeighbour(*n);
        ++x;
        if (auto n = graph.find(Tile{++x, y}); n != graph.end())
            node.AddNeighbour(*n);
        --x;
        if (auto n = graph.find(Tile{x, ++y}); n != graph.end())
            node.AddNeighbour(*n);
    }
public:
    Graph(const std::map<Tile, std::unique_ptr<Unit>>& tiles) {
        for (const auto& elem : tiles)
            if (elem.second->Type() == UnitType::Empty)
                graph.emplace(elem.first);
    }

    std::vector<Tile> GetShortestPath(const Tile& origin, const Tile& dest) const {
        auto graf = this->graph;
        auto& start = *graf.emplace(origin).first;
        auto& end = *graf.emplace(dest).first;
        for (const auto& node : graf)
            AddNeighbours(graf, node);
        std::unordered_set<Tile, Tile::Hash> visited, queued;
        std::queue<std::reference_wrapper<const Node>> queue{{start}};
        std::vector<Node> paths;
        while (!queue.empty()) {
            auto cur = std::move(queue.front());
            queue.pop();
            visited.emplace(cur);
            for (auto& child : cur.get().Neighbours()) {
                if (visited.count(child))
                    continue;
                if (end == child) {
                    std::vector<Tile> ret;
                    for (auto chk = std::cref(cur); start != chk; chk = chk.get().GetParent().value())
                        ret.emplace_back(chk);
                    return ret;
                }
                if (!queued.count(child)) {
                    queue.emplace(child);
                    queued.emplace(child);
                    child.get().SetParent(cur);
                }
                /*auto curParent = child.get().GetParent();
                if (!curParent)
                    child.get().SetParent(cur);*/
            }
        }
        return {};
    }
};

class Combatant;

class IMap {
public:
    virtual const Tile& MoveCombatant(const Combatant&, const Tile&) = 0;
    virtual void Remove(const Combatant&) = 0;
    virtual std::vector<std::reference_wrapper<Combatant>>& GetElves() = 0;
    virtual std::vector<std::reference_wrapper<Combatant>>& GetGoblins() = 0;
    virtual const std::map<Tile, std::unique_ptr<Unit>>& GetMap() const = 0;
    virtual const Graph& GetGraph() const = 0;
    virtual ~IMap() = default;
};

class Combatant : public Unit {
    int health = 200;
    int power = 3;
    std::reference_wrapper<const Tile> curTile;

    virtual std::vector<std::reference_wrapper<Combatant>>& Enemies(IMap&) = 0;

    uint16_t Distance(const Combatant& other) const noexcept {
        return curTile.get().Distance(other.curTile);
    }

    void Attack(Combatant& target, IMap& map) {
        target.health -= power;
        if (target.health <= 0)
            map.Remove(target);
    }

    void MoveTo(const Tile& tile, IMap& map) {
        curTile = map.MoveCombatant(*this, tile);
    }
public:
	Combatant(const Tile& curTile) : curTile{curTile} {}

	int Health() const final override {
		return health;
	}
	int Power() const final override {
		return power;
	}
	bool operator<(const Combatant& other) const noexcept {
		return curTile.get() < other.curTile;
	}

	bool operator==(const Combatant& other) const noexcept {
	    return this == &other || (health == other.health && power == other.power && curTile.get() == other.curTile);
	}

	const Tile& CurTile() const noexcept {
        return curTile;
    }

    template<class T>
    bool TryAttack(T& targets, IMap& map) {
        auto end = std::stable_partition(targets.begin(), targets.end(), [&] (const auto& e) {
            return Distance(e) == 1;
        });
        if (end == targets.begin())
            return false;
        std::stable_sort(targets.begin(), end, [] (const auto& a, const auto& b) {
            return a.get().Health() < b.get().Health();
        });
        Attack(targets.front(), map);
        return true;
    }

    bool Tick(IMap& map) {
        auto targets = Enemies(map);
        if (targets.empty())
            return false;
        if (TryAttack(targets, map))
            return true;
        auto& graph = map.GetGraph();
        std::sort(targets.begin(), targets.end(), [&] (const auto& a, const auto& b) {
            auto pathA = graph.GetShortestPath(curTile, a.get().curTile);
            if (pathA.empty())
                return false;
            auto pathB = graph.GetShortestPath(curTile, b.get().curTile);
            if (pathB.empty())
                return true;
            return pathA.size() < pathB.size();
        });
        auto shortestPath = graph.GetShortestPath(curTile, targets.front().get().curTile);
        if (!shortestPath.empty()) {
            MoveTo(shortestPath.back(), map);
            TryAttack(targets, map);
        }
        return true;
    }
};

class Elf final : public Combatant {
public:
	using Combatant::Combatant;
	UnitType Type() const noexcept override {
		return UnitType::Elf;
	}

    std::vector<std::reference_wrapper<Combatant>>& Enemies(IMap& map) override {
	    return map.GetGoblins();
	}
};

class Goblin final : public Combatant {
public:
	using Combatant::Combatant;
	UnitType Type() const noexcept override {
		return UnitType::Goblin;
	}

    std::vector<std::reference_wrapper<Combatant>>& Enemies(IMap& map) override {
        return map.GetElves();
    }
};

class Wall final : public Unit {
public:
	int Health() const override { throw std::logic_error{"Cannot get health of a wall"}; }
	int Power() const override { throw std::logic_error{"Cannot get power of a wall" }; }
	UnitType Type() const noexcept override {
		return UnitType::Wall;
	}
};

class Empty final : public Unit {
public:
	int Health() const override { throw std::logic_error{"Cannot get health of Empty"}; }
	int Power() const override { throw std::logic_error{"Cannot get power of Empty"}; }
	UnitType Type() const noexcept override {
		return UnitType::Empty;
	}
};

class Map : public IMap {
	std::vector<std::reference_wrapper<Combatant>> elves;
	std::vector<std::reference_wrapper<Combatant>> goblins;
	std::list<std::reference_wrapper<Combatant>> combatants;
	std::map<Tile, std::unique_ptr<Unit>> map;
	Graph graph{map};

	decltype(map) BuildMap(std::istream& in) {
		decltype(map) ret;
		auto x = 0;
		auto y = 0;
		while (in) {
			switch (in.get()) {
				case '#':
					ret.emplace(std::piecewise_construct, std::forward_as_tuple(x, y), std::forward_as_tuple(std::make_unique<Wall>()));
					break;
				case '.':
					ret.emplace(std::piecewise_construct, std::forward_as_tuple(x, y), std::forward_as_tuple(std::make_unique<Empty>()));
					break;
				case 'E': {
						auto e = ret.emplace(std::piecewise_construct, std::forward_as_tuple(x, y), std::forward_as_tuple(nullptr));
						auto elfPtr = std::make_unique<Elf>(e.first->first);
						auto& elf = *elfPtr;
						e.first->second = std::move(elfPtr);
						elves.emplace_back(elf);
						combatants.emplace_back(elves.back());
					}
					break;
				case 'G': {
						auto g = ret.emplace(std::piecewise_construct, std::forward_as_tuple(x, y), std::forward_as_tuple(nullptr));
						auto goblinPtr = std::make_unique<Goblin>(g.first->first);
						auto& goblin = *goblinPtr;
						g.first->second = std::move(goblinPtr);
						goblins.emplace_back(goblin);
						combatants.emplace_back(goblins.back());
					}
					break;
				case '\n':
					++y;
					x = 0;
					continue;
			}
			++x;
		}
		return ret;
	}
public:
	Map(std::istream& in) : map{BuildMap(in)} {}

	const Tile& MoveCombatant(const Combatant& combatant, const Tile& tile) override {
	    auto dest = map.find(tile);
	    auto src = map.find(combatant.CurTile());
	    if (dest == map.end() || src == map.end())
	        throw std::logic_error{"Non existent location"};
	    auto& ret = dest->first;
	    if (dest->second->Type() != UnitType::Empty)
	        throw std::logic_error{"Not an empty spot!"};
	    std::swap(src->second, dest->second);
	    return ret;
	}

	void Remove(const Combatant& c) override {
	    auto tgt = map.find(c.CurTile());
	    if (tgt == map.end())
	        throw std::logic_error{"Trying to remove non-existent entity"};
	    auto find = [&c] (const auto& e) { return c == e; };
	    if (c.Type() == UnitType::Elf)
	        elves.erase(std::find_if(elves.begin(), elves.end(), find));
	    else if (c.Type() == UnitType::Goblin)
	        goblins.erase(std::find_if(goblins.begin(), goblins.end(), find));
	    combatants.erase(std::find_if(combatants.begin(), combatants.end(), find));
	    tgt->second = std::make_unique<Empty>();
	}

	std::vector<std::reference_wrapper<Combatant>>& GetElves() override {
	    return elves;
	}

	std::vector<std::reference_wrapper<Combatant>>& GetGoblins() override {
	    return goblins;
	}

	const decltype(combatants)& GetCombatants() const {
	    return combatants;
	}

    const std::map<Tile, std::unique_ptr<Unit>>& GetMap() const override {
	    return map;
	}

	const Graph& GetGraph() const override {
	    return graph;
	}

	bool DoTick() {
        auto sorter = [] (const auto& a, const auto& b) { return a.get() < b; };
	    for (auto& combatant : combatants) {
	        if (!combatant.get().Tick(*this))
	            return false;
	        graph = Graph{map};
            std::sort(elves.begin(), elves.end(), sorter);
            std::sort(goblins.begin(), goblins.end(), sorter);
	    }
	    combatants.sort(sorter);
	    return true;
	}

	friend std::ostream& operator<<(std::ostream& o, const Map& map) {
	    auto y = 0;
	    for (const auto& elem : map.map) {
	        auto& tile = elem.first;
	        if (tile.Y() > y) {
	            o << '\n';
	            ++y;
	        }
	        switch (elem.second->Type()) {
	            case UnitType::Wall:
	                o << '#';
	                break;
	            case UnitType::Empty:
	                o << '.';
	                break;
	            case UnitType::Elf:
	                o << 'E';
	                break;
	            case UnitType::Goblin:
	                o << 'G';
	        }
	    }
	    o << '\n';
	    for (const auto& g : map.goblins) {
	        o << g.get().Health() << '\n';
	    }
	    return o;
	}
};

int main(int argc, const char* argv[]) {
	std::ifstream file{argc == 2 ? argv[1] : "input.txt"};
	Map map{file};
    auto round = 0;
	while (map.DoTick())
	    ++round;
    auto totalHealth = 0;
    for (const auto& c : map.GetCombatants()) {
        totalHealth += c.get().Health();
    }
    std::cout << "Answer: " << totalHealth * round << '\n';
}
