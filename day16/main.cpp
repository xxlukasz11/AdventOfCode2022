#include "../common/pch.h"
#include <iostream>
#include <cstdint>
#include <string>
#include <algorithm>
#include <numeric>
#include <utility>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <unordered_set>

struct Valve {
	std::string id;
	int flowRate{ 0 };
	std::vector<std::string> neighborIds;
};

using DataType = std::unordered_map<std::string, Valve>;

DataType read() {
	common::FileReader reader("input.txt");
	DataType data;
	for (std::string line; reader.nextLine(line);) {
		std::string id = line.substr(line.find("Valve") + 6, 2);
		auto& valve = data[id];
		valve.id = id;
		valve.flowRate = std::stoi(line.substr(line.find('=') + 1));

		auto neighborsStr = line.substr(line.find("valve") + 6);
		common::replaceAll(neighborsStr, ",", "");
		valve.neighborIds = common::parseArray<std::string>(neighborsStr);
	}
	return data;
}

struct Node {
	std::string id;
	int distance;
};

bool operator<(const Node& n1, const Node& n2) {
	if (n1.distance == n2.distance) {
		return n1.id < n2.id;
	}
	return n1.distance < n2.distance;
}

using DijkstraMap = std::map<std::string, std::pair<int, std::vector<std::string>>>;

int getExistingValueOrMax(DijkstraMap& map, const std::string& key) {
	auto found = map.find(key);
	if (found == map.end()) {
		auto max = 100000;
		map[key] = { max, {} };
		return max;
	}
	return found->second.first;
}

std::vector<std::string> dijkstra(const DataType& data, const std::string& startId, const std::string& endId) {
	DijkstraMap distances;
	std::set<Node> nodes;
	nodes.insert(Node{ startId, 0 });
	distances[startId] = { 0, {startId} };

	while (!nodes.empty()) {
		auto node = *nodes.begin();
		nodes.erase(nodes.begin());
		auto allNeighbors = data.find(node.id)->second.neighborIds;
		for (const auto& neighbor : allNeighbors) {
			int nodeDistance = getExistingValueOrMax(distances, node.id);
			int neighborDistance = getExistingValueOrMax(distances, neighbor);
			int newNeighborDistance = nodeDistance + 1;
			if (neighborDistance > newNeighborDistance) {
				auto path = distances[node.id].second;
				path.push_back(neighbor);
				auto found = nodes.find(Node{ neighbor, neighborDistance });
				if (found != nodes.end()) {
					nodes.erase(found);
				}
				nodes.insert(Node{ neighbor, newNeighborDistance });
				distances[neighbor] = { newNeighborDistance, path };
			}
		}
	}

	return distances[endId].second;
}

using DistancesMap = std::unordered_map<std::string, std::unordered_map<std::string, const std::vector<std::string>>>;

class Actor {
public:
	Actor(const std::string& initialValveId) : currentValveId(initialValveId) {}

	void tick(int currentTime) {
		bool shouldWait = blocked && currentTime < blockedUntil;
		if (shouldWait) {
			return;
		}
		blocked = false;

		if (moving) {
			moving = false;
			currentValveId = targetValveId;
		}
	}

	bool isReady() const {
		return !blocked;
	}

	bool isMoving() const {
		return moving;
	}

	const std::string& getCurrentValveId() const {
		return currentValveId;
	}

	const std::string& getTargetValveId() const {
		return targetValveId;
	}

	void movingTo(const std::string& valveId, int arrivalTime) {
		blocked = true;
		moving = true;
		blockedUntil = arrivalTime;
		targetValveId = valveId;
	}

	void openingValve(int endTime) {
		blocked = true;
		blockedUntil = endTime;
	}

private:
	bool blocked{ false };
	int blockedUntil{ 0 };

	std::string currentValveId;

	bool moving{ false };
	std::string targetValveId;
};

class Trawerse {
public:
	Trawerse(DataType& data) : data(data) {
		for (const auto& [id, valve] : data) {
			if (valve.flowRate > 0) {
				nonZeroValves.push_back(id);
			}
		}

		for (int i = 0; i < nonZeroValves.size(); ++i) {
			for (int j = i + 1; j < nonZeroValves.size(); ++j) {
				const auto& src = nonZeroValves[i];
				const auto& tgt = nonZeroValves[j];
				auto distance = dijkstra(data, src, tgt);
				distances[src].insert({ tgt, distance});
				std::reverse(distance.begin(), distance.end());
				distances[tgt].insert({ src, distance });
			}
		}

		const auto start = "AA";
		for (int i = 0; i < nonZeroValves.size(); ++i) {
			const auto& tgt = nonZeroValves[i];
			auto distance = dijkstra(data, start, tgt);
			distances[start].insert({ tgt, distance });
		}
	}

	int getMaxScore() const {
		return maxScore;
	}

	void startPart1(int totalTime) {
		this->totalTime = totalTime;
		this->maxScore = 0;
		std::set<std::string> openedValves;
		trawerse(0, totalTime, "AA", openedValves);
	}

	void startPart2(int totalTime) {
		this->totalTime = totalTime;
		this->maxScore = 0;
		const auto initialValveId = "AA";
		Actor me(initialValveId);
		Actor elephant(initialValveId);
		std::unordered_set<std::string> notOpenedValves(nonZeroValves.begin(), nonZeroValves.end());
		nextMinuteWithElephant(0, 0, notOpenedValves, me, elephant);
	}

	void trawerse(int currentScore, int minutesLeft, const std::string& valveId, std::set<std::string> openedValves) {
		auto& valve = data[valveId];

		if (valve.flowRate > 0) {
			minutesLeft -= 1;
			currentScore += valve.flowRate * minutesLeft;
			openedValves.insert(valve.id);
		}

		auto& distancesFromCurrent = distances[valve.id];
		std::vector<std::string> possibleTargets;
		for (const auto& targetId : nonZeroValves) {
			if (!openedValves.contains(targetId)) {
				possibleTargets.push_back(targetId);
			}
		}

		if (possibleTargets.empty()) {
			addNewScore(currentScore);
			return;
		}

		bool hasContinuation = false;
		for (const auto& targetId : possibleTargets) {
			auto path = distancesFromCurrent[targetId];
			auto distance = path.size() - 1;
			auto timeLeftAfterArrival = minutesLeft - distance;
			auto timeLeftAfterOpening = timeLeftAfterArrival - 1;
			if (timeLeftAfterOpening >= 0) {
				hasContinuation = true;
				trawerse(currentScore, timeLeftAfterArrival, targetId, openedValves);
			}
		}

		if (!hasContinuation) {
			addNewScore(currentScore);
			return;
		}
	}

	std::vector<Actor> createNewActors(const Actor& actor, int timePassed, const std::unordered_set<std::string>& notOpenedValves) {
		std::vector<Actor> newActors;
		if (!actor.isReady()) {
			return newActors;
		}

		auto& distancesFromActor = distances[actor.getCurrentValveId()];
		for (const auto& targetId : notOpenedValves) {
			auto path = distancesFromActor[targetId];
			auto distance = path.size() - 1;
			auto timeOfArrival = timePassed + distance;
			auto timeOfOpening = timeOfArrival + 1;
			bool pathContainsUnopenedValves = std::any_of(std::next(path.begin()), std::prev(path.end()), [&notOpenedValves](auto&& id) {
				return notOpenedValves.contains(id);
			});
			if (timeOfOpening <= totalTime && !pathContainsUnopenedValves) {
				Actor newActor = actor;
				newActor.movingTo(targetId, timeOfArrival);
				newActors.push_back(newActor);
			}
		}
		return newActors;
	}

	void nextMinuteWithElephant(int currentScore, int timePassed, std::unordered_set<std::string> notOpenedValves, Actor me, Actor elephant) {
		if (timePassed >= totalTime) {
			addNewScore(currentScore);
			return;
		}
		
		me.tick(timePassed);
		elephant.tick(timePassed);

		auto& myValve = data[me.getCurrentValveId()];
		auto& elephantValve = data[elephant.getCurrentValveId()];

		const auto timeOfOpening = timePassed + 1;
		if (me.isReady() && myValve.flowRate > 0 && notOpenedValves.contains(myValve.id)) {
			currentScore += myValve.flowRate * (totalTime - timeOfOpening);
			notOpenedValves.erase(myValve.id);
			me.openingValve(timePassed + 1);
		}

		if (elephant.isReady() && elephantValve.flowRate > 0 && notOpenedValves.contains(elephantValve.id)) {
			currentScore += elephantValve.flowRate * (totalTime - timeOfOpening);
			notOpenedValves.erase(elephantValve.id);
			elephant.openingValve(timePassed + 1);
		}

		if (!me.isReady() && !elephant.isReady()) {
			nextMinuteWithElephant(currentScore, timePassed + 1, notOpenedValves, me, elephant);
		}

		if (notOpenedValves.empty()) {
			addNewScore(currentScore);
			return;
		}

		std::vector<Actor> newElephants = createNewActors(elephant, timePassed, notOpenedValves);
		std::vector<Actor> newMes = createNewActors(me, timePassed, notOpenedValves);

		if (newMes.empty()) {
			newMes.push_back(me);
		}

		if (newElephants.empty()) {
			newElephants.push_back(elephant);
		}

		for (const auto& newMe : newMes) {
			for (const auto& newElephant : newElephants) {
				const bool bothAreMoving = newElephant.isMoving() && newMe.isMoving();
				const bool haveTheSameTarget = newMe.getTargetValveId() == newElephant.getTargetValveId();
				if (notOpenedValves.size() > 1 && bothAreMoving && haveTheSameTarget) {
					continue;
				}
				nextMinuteWithElephant(currentScore, timePassed + 1, notOpenedValves, newMe, newElephant);
			}
		}
	}

private:
	void addNewScore(int score) {
		if (score > maxScore) {
			maxScore = score;
		}
	}

	DataType& data;
	std::vector<std::string> nonZeroValves;
	DistancesMap distances;
	int totalTime{ 0 };
	int maxScore{ 0 };
};

int partOne(DataType data) {
	auto trawerse = Trawerse(data);
	trawerse.startPart1(30);
	return trawerse.getMaxScore();
}

int partTwo(DataType data) {
	auto trawerse = Trawerse(data);
	trawerse.startPart2(26);
	return trawerse.getMaxScore();
}

int main(int argc, char** argv) {
	const auto data = read();
	common::measureAndPrint("Part 1", partOne, data);
	common::measureAndPrint("Part 2", partTwo, data);
	return 0;
}