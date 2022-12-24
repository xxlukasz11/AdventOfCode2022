#include "../common/pch.h"
#include <cstdint>
#include <string>
#include <numeric>
#include <vector>
#include <unordered_set>

struct Point {
	int x;
	int y;

	bool operator==(const Point& point) const {
		return x == point.x && y == point.y;
	}

	bool operator<(const Point& point) const {
		return x == point.x ? y < point.y : x < point.x;
	}
};

namespace std {
template <> struct hash<Point> {
	size_t operator()(const Point& point) const {
		return std::hash<int>()(point.x) ^ std::hash<int>()(point.y);
	}
};
}

struct Measurement {
	Point sensor;
	Point beacon;
	int distance;
};

using DataType = std::vector<Measurement>;

int manhattanDistance(const Point& left, const Point& right) {
	return abs(left.x - right.x) + abs(left.y - right.y);
}

DataType read() {
	common::FileReader reader("input.txt");
	DataType data;
	for (std::string line; reader.nextLine(line);) {
		Measurement meas;
		auto eqPos = line.find('=');
		line = line.substr(eqPos + 1);
		meas.sensor.x = std::stoi(line);

		eqPos = line.find('=');
		line = line.substr(eqPos + 1);
		meas.sensor.y = std::stoi(line);

		eqPos = line.find('=');
		line = line.substr(eqPos + 1);
		meas.beacon.x = std::stoi(line);

		eqPos = line.find('=');
		line = line.substr(eqPos + 1);
		meas.beacon.y = std::stoi(line);

		meas.distance = manhattanDistance(meas.sensor, meas.beacon);
		data.push_back(meas);
	}
	return data;
}

int calculateMinSensorX(const DataType& data) {
	return std::min_element(data.begin(), data.end(), [](auto&& left, auto&& right) {
		return left.sensor.x < right.sensor.x;
	})->sensor.x;
}

int calculateMaxSensorX(const DataType& data) {
	return std::max_element(data.begin(), data.end(), [](auto&& left, auto&& right) {
		return left.sensor.x < right.sensor.x;
	})->sensor.x;
}

int calculateMaxDistance(const DataType& data) {
	return std::max_element(data.begin(), data.end(), [](auto&& left, auto&& right) {
		return left.distance < right.distance;
	})->distance;
}

int partOne(const DataType& data) {
	int minSensorX = calculateMinSensorX(data);
	int maxSensorX = calculateMaxSensorX(data);
	int maxSensorRange = calculateMaxDistance(data);
	const auto minX = minSensorX - maxSensorRange;
	const auto maxX = maxSensorX + maxSensorRange;

	std::unordered_set<Point> occupied;
	for (const auto& [sensor, beacon, _] : data) {
		occupied.insert(sensor);
		occupied.insert(beacon);
	}

	const int y = 2000000;
	int cannotContainBeaconCount = 0;
	for (int x = minX; x <= maxX; ++x) {
		Point pos{ x, y };
		for (const auto& [sensor, _, distance] : data) {
			if (manhattanDistance(sensor, pos) <= distance && !occupied.contains(pos)) {
				++cannotContainBeaconCount;
				break;
			}
		}
	}

	return cannotContainBeaconCount;
}

struct Line {
	Point start;
	Point end;
};

std::vector<Point> findIntersectionNeighbors(const Line& left, const Line& right) {
	int64_t a1 = left.end.y - left.start.y;
	int64_t b1 = left.start.x - left.end.x;
	int64_t c1 = a1 * left.start.x + b1 * left.start.y;

	int64_t a2 = right.end.y - right.start.y;
	int64_t b2 = right.start.x - right.end.x;
	int64_t c2 = a2 * right.start.x + b2 * right.start.y;

	int64_t determinant = a1 * b2 - a2 * b1;

	std::vector<Point> neighbors;
	if (determinant == 0) {
		return neighbors;
	}

	constexpr int64_t multiplier = 2;
	int64_t x = ((b2 * c1 - b1 * c2) * multiplier) / determinant;
	int64_t y = ((a1 * c2 - a2 * c1) * multiplier) / determinant;

	int64_t minX = x / multiplier;
	int64_t minY = y / multiplier;

	neighbors.emplace_back(minX, minY);

	if (minX * multiplier != x) {
		int64_t maxX = minX + 1;
		int64_t maxY = minY + 1;
		neighbors.emplace_back(minX, maxY);
		neighbors.emplace_back(maxX, maxY);
		neighbors.emplace_back(maxX, minY);
	}

	return neighbors;
}

uint64_t partTwo(const DataType& data) {
	const auto maxRange = 4000000;
	const auto minRange = 0;

	std::vector<Line> allLines;
	for (const auto& [sensor, beacon, distance] : data) {
		const Point top = { sensor.x, sensor.y - distance };
		const Point bottom = { sensor.x, sensor.y + distance };
		const Point left = { sensor.x - distance, sensor.y };
		const Point right = { sensor.x + distance, sensor.y };
		allLines.emplace_back(top, right);
		allLines.emplace_back(top, left);
		allLines.emplace_back(left, bottom);
		allLines.emplace_back(right, bottom);
	}

	Point topLeft = { minRange - 1, minRange - 1 };
	Point topRight = { maxRange + 1, minRange - 1 };
	Point bottomLeft = { minRange - 1, maxRange + 1 };
	Point bottomRight = { maxRange + 1, maxRange + 1 };
	allLines.emplace_back(topLeft, topRight);
	allLines.emplace_back(topRight, bottomRight);
	allLines.emplace_back(bottomRight, bottomLeft);
	allLines.emplace_back(bottomLeft, topLeft);

	const auto allLinesCount = allLines.size();
	std::unordered_set<Point> intersections;
	for (int i = 0; i < allLinesCount; ++i) {
		for (int j = i + 1; j < allLinesCount; ++j) {
			const auto& left = allLines[i];
			const auto& right = allLines[j];
			const auto neighbors = findIntersectionNeighbors(left, right);
			intersections.insert(neighbors.begin(), neighbors.end());
		}
	}

	std::unordered_set<Point> pointsToCheck;
	for (const auto& intersection : intersections) {
		std::vector<Point> possibilities;
		for (int x = intersection.x - 1; x <= intersection.x + 1; ++x) {
			for (int y = intersection.y - 1; y <= intersection.y + 1; ++y) {
				if (x == intersection.x && y == intersection.y) {
					continue;
				}
				possibilities.emplace_back(x, y);
			}
		}
		for (const auto& point : possibilities) {
			if (point.x < minRange || point.x > maxRange) {
				continue;
			}
			if (point.y < minRange || point.y > maxRange) {
				continue;
			}
			pointsToCheck.insert(point);
		}
	}

	for (const auto& point : pointsToCheck) {
		bool beaconCanBeHere = true;
		for (const auto & [sensor, _, distance] : data) {
			if (manhattanDistance(point, sensor) <= distance) {
				beaconCanBeHere = false;
				break;
			}
		}
		if (beaconCanBeHere) {
			return point.x * 4000000ULL + point.y;
		}
	}

	return -1;
}

int main(int argc, char** argv) {
	const auto data = read();
	common::measureAndPrint("Part 1", partOne, data);
	common::measureAndPrint("Part 2", partTwo, data);
	return 0;
}