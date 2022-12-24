#include "../common/pch.h"
#include <string>
#include <algorithm>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <array>

using Point = common::Point<int>;

struct Line {
	std::vector<Point> points;
};

using DataType = std::vector<Line>;

std::vector<std::string> splitByDelimiter(std::string line, const std::string& delimiter) {
	int pos;
	std::vector<std::string> result;
	while ((pos = line.find(delimiter)) != std::string::npos && pos != line.size() - delimiter.size()) {
		result.push_back(line.substr(0, pos));
		line = line.substr(pos + delimiter.size());
	}
	if (!line.empty()) {
		result.push_back(line);
	}
	return result;
}

DataType read() {
	common::FileReader reader("input.txt");
	DataType data;
	for (std::string line; reader.nextLine(line);) {
		auto pointsToParse = splitByDelimiter(line, " -> ");
		Line parsedLine;
		std::transform(pointsToParse.begin(), pointsToParse.end(), std::back_inserter(parsedLine.points), [](auto&& pointToParse) {
			std::istringstream stream(pointToParse);
			Point point;
			stream >> point.x;
			stream.ignore();
			stream >> point.y;
			return point;
		});
		data.push_back(parsedLine);
	}
	return data;
}

void fillMap(std::unordered_set<Point>& map, const Line& line) {
	const auto& points = line.points;
	for (int i = 1; i < points.size(); ++i) {
		const auto& start = points[i - 1];
		const auto& end = points[i];
		if (start.x == end.x) {
			auto yDiff = end.y - start.y;
			auto numPoints = abs(yDiff) + 1;
			auto increment = yDiff / abs(yDiff);
			for (int j = 0; j < numPoints; ++j) {
				auto y = start.y + j * increment;
				map.insert({ start.x, y });
			}
		} else {
			auto xDiff = end.x - start.x;
			auto numPoints = abs(xDiff) + 1;
			auto increment = xDiff / abs(xDiff);
			for (int j = 0; j < numPoints; ++j) {
				auto x = start.x + j * increment;
				map.insert({ x, start.y });
			}
		}
	}
}

bool simulateSand(std::unordered_set<Point>& occupiedPlaces, int maxY) {
	Point sand{ 500, 0 };
	while (sand.y < maxY) {
		int newYLevel = sand.y + 1;
		const std::array<Point, 3> possibilities{ { {sand.x, newYLevel}, {sand.x - 1, newYLevel}, {sand.x + 1, newYLevel} } };
		bool moved = false;
		for (const auto& possibility : possibilities) {
			if (!occupiedPlaces.contains(possibility)) {
				sand = possibility;
				moved = true;
				break;
			}
		}
		if (!moved) {
			occupiedPlaces.insert(sand);
			return true;
		}
	}
	return false;
}

bool simulateSandWithFloor(std::unordered_set<Point>& occupiedPlaces, int maxY) {
	Point sand{ 500, 0 };
	if (occupiedPlaces.contains(sand)) {
		return false;
	}

	const auto floorLevel = maxY + 2;
	while (sand.y < floorLevel - 1) {
		auto newYLevel = sand.y + 1;
		const std::array<Point, 3> possibilities{ { {sand.x, newYLevel}, {sand.x - 1, newYLevel}, {sand.x + 1, newYLevel} } };
		bool moved = false;
		for (const auto& possibility : possibilities) {
			if (!occupiedPlaces.contains(possibility)) {
				sand = possibility;
				moved = true;
				break;
			}
		}
		if (!moved) {
			break;
		}
	}
	occupiedPlaces.insert(sand);
	return true;
}

int calculateMaxY(const std::unordered_set<Point>& occupiedPlaces) {
	int maxY = 0;
	for (const auto& point : occupiedPlaces) {
		if (point.y > maxY) {
			maxY = point.y;
		}
	}
	return maxY;
}

int partOne(const DataType& data) {
	std::unordered_set<Point> occupiedPlaces;
	for (const auto& line : data) {
		fillMap(occupiedPlaces, line);
	}

	const auto maxY = calculateMaxY(occupiedPlaces);

	int restedSand = 0;
	while (simulateSand(occupiedPlaces, maxY)) {
		++restedSand;
	}

	return restedSand;
}

int partTwo(const DataType& data) {
	std::unordered_set<Point> occupiedPlaces;
	for (const auto& line : data) {
		fillMap(occupiedPlaces, line);
	}

	const auto maxY = calculateMaxY(occupiedPlaces);

	int restedSand = 0;
	while (simulateSandWithFloor(occupiedPlaces, maxY)) {
		++restedSand;
	}

	return restedSand;
}

int main(int argc, char** argv) {
	const auto data = read();
	common::measureAndPrint("Part 1", partOne, data);
	common::measureAndPrint("Part 2", partTwo, data);
	return 0;
}