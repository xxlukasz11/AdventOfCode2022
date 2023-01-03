#include "../common/pch.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <numeric>
#include <utility>
#include <vector>
#include <array>
#include <cmath>
#include <regex>

using common::Dir;
using Point = common::Point<int>;

struct Command {
	bool isDirChange{ false };
	int value{ 0 };
	Dir dir{ 0 , 0 };
};

enum GridValue {
	TILE,
	EMPTY
};

struct EndPoint {
	Point pos;
	Dir dir;
};

struct Connection {
	EndPoint src;
	EndPoint tgt;
};

constexpr int GRID_SIZE = 4;

using MapType = std::vector<std::string>;
using CommandList = std::vector<Command>;
using GridType = std::vector<std::vector<GridValue>>;
using DataType = std::pair<MapType, CommandList>;

Dir parseDir(char character) {
	if (character == 'R') {
		return Dir::RIGHT;
	}
	return Dir::LEFT;
}

CommandList parseCommands(const std::string& input) {
	std::regex pattern("([RL]*)(\\d+)");
	std::sregex_iterator start(input.begin(), input.end(), pattern);
	std::sregex_iterator end;

	CommandList commands;
	for (auto it = start; it != end; ++it) {
		const auto dirStr = it->str(1);
		if (!dirStr.empty()) {
			Command cmd;
			cmd.isDirChange = true;
			cmd.dir = parseDir(dirStr.front());
			commands.push_back(cmd);
		}

		const auto valueStr = it->str(2);
		if (!valueStr.empty()) {
			Command cmd;
			cmd.value = std::stoi(valueStr);
			commands.push_back(cmd);
		}
	}
	return commands;
}

DataType read() {
	common::FileReader reader("input.txt");
	MapType map;
	for (std::string line; reader.nextLine(line);) {
		if (line.empty()) {
			break;
		}
		map.push_back(line);
	}
	CommandList commands = parseCommands(reader.nextLine());
	DataType data{ std::move(map), std::move(commands) };
	return data;
}

class MapWalker {
public:
	MapWalker(const MapType& map) : map(map), currentDir(Dir::RIGHT) {
		calculateRowOffsets();
		calculateColumnRanges();
		currentPos = {rowOffsets.front(), 0};
	}

	void applyCommand(const Command& command) {
		if (command.isDirChange) {
			changeDir(command.dir);
			return;
		}

		const auto moves = command.value;
		advanceInCurrentDir(moves);
	}

	const Point& getPosition() const {
		return currentPos;
	}

	const Dir& getDirection() const {
		return currentDir;
	}

private:
	void calculateRowOffsets() {
		for (const auto& row : map) {
			auto offset = row.find_first_not_of(' ');
			rowOffsets.push_back(offset);
		}
	}

	void calculateColumnRanges() {
		const int maxRange = map.size();
		const int minRange = 0;
		const std::pair<int, int> initialRange(maxRange, minRange);
		for (int rowIdx = 0; rowIdx < map.size(); ++rowIdx) {
			auto offset = rowOffsets[rowIdx];
			const auto& row = map[rowIdx];
			for (int columnIdx = offset; columnIdx < row.size(); ++columnIdx) {
				if (columnIdx >= columnRanges.size()) {
					columnRanges.resize(columnIdx + 1, initialRange);
				}

				if (rowIdx < columnRanges[columnIdx].first) {
					columnRanges[columnIdx].first = rowIdx;
				}

				if (rowIdx > columnRanges[columnIdx].second) {
					columnRanges[columnIdx].second = rowIdx;
				}
			}
		}
	}

	void changeDir(const Dir& commandDir) {
		if (commandDir == Dir::RIGHT) {
			currentDir = { -currentDir.y, currentDir.x };
		} else {
			currentDir = { currentDir.y, -currentDir.x };
		}
	}

	void advanceInCurrentDir(int moves) {
		while (moves-- > 0 && advanceInCurrentDirOneStep());
	}

	bool advanceInCurrentDirOneStep() {
		Point newPos = currentPos;
		newPos.x += currentDir.x;
		newPos.y += currentDir.y;

		if (currentDir.y != 0) {
			auto columnRange = columnRanges[newPos.x];
			if (newPos.y > columnRange.second) {
				newPos.y = columnRange.first;
			}

			if (newPos.y < columnRange.first) {
				newPos.y = columnRange.second;
			}
		}

		const auto& newRow = map[newPos.y];
		const auto& rowOffset = rowOffsets[newPos.y];

		if (currentDir.x != 0) {
			if (newPos.x < rowOffset) {
				newPos.x = newRow.size() - 1;
			}

			if (newPos.x >= newRow.size()) {
				newPos.x = rowOffset;
			}
		}

		if (newRow[newPos.x] == '#') {
			return false;
		}
		currentPos = newPos;
		return true;
	}

	const MapType& map;
	Dir currentDir;
	std::vector<int> rowOffsets;
	std::vector<std::pair<int ,int>> columnRanges;
	Point currentPos;
};

class CubeWalker {
public:
	CubeWalker(const MapType& map, const std::vector<Connection>& connections, int edgeLength) : map(map),
			connections(connections), edgeLength(edgeLength), currentDir(Dir::RIGHT) {
		calculateRowOffsets();
		calculateColumnRanges();
		currentPos = { rowOffsets.front(), 0 };
	}

	void applyCommand(const Command& command) {
		if (command.isDirChange) {
			changeDir(command.dir);
			return;
		}

		const auto moves = command.value;
		advanceInCurrentDir(moves);
	}

	const Point& getPosition() const {
		return currentPos;
	}

	const Dir& getDirection() const {
		return currentDir;
	}

private:
	void calculateRowOffsets() {
		for (const auto& row : map) {
			auto offset = row.find_first_not_of(' ');
			rowOffsets.push_back(offset);
		}
	}

	void calculateColumnRanges() {
		const int maxRange = map.size();
		const int minRange = 0;
		const std::pair<int, int> initialRange(maxRange, minRange);
		for (int rowIdx = 0; rowIdx < map.size(); ++rowIdx) {
			auto offset = rowOffsets[rowIdx];
			const auto& row = map[rowIdx];
			for (int columnIdx = offset; columnIdx < row.size(); ++columnIdx) {
				if (columnIdx >= columnRanges.size()) {
					columnRanges.resize(columnIdx + 1, initialRange);
				}

				if (rowIdx < columnRanges[columnIdx].first) {
					columnRanges[columnIdx].first = rowIdx;
				}

				if (rowIdx > columnRanges[columnIdx].second) {
					columnRanges[columnIdx].second = rowIdx;
				}
			}
		}
	}

	void changeDir(const Dir& commandDir) {
		if (commandDir == Dir::RIGHT) {
			currentDir = { -currentDir.y, currentDir.x };
		} else {
			currentDir = { currentDir.y, -currentDir.x };
		}
	}

	void advanceInCurrentDir(int moves) {
		while (moves-- > 0 && advanceInCurrentDirOneStep());
	}

	int claculateOffsetOnANewTile(const EndPoint& endPoint, int offset, const Dir& srcDir) {
		const auto& [tgtPos, tgtDir] = endPoint;

		bool swapOffset = false;
		if (srcDir == tgtDir) {
			swapOffset = true;
		}
		if (srcDir == Dir::TOP && tgtDir == Dir::RIGHT) {
			swapOffset = true;
		}
		if (srcDir == Dir::LEFT && tgtDir == Dir::BOTTOM) {
			swapOffset = true;
		}
		if (srcDir == Dir::RIGHT && tgtDir == Dir::TOP) {
			swapOffset = true;
		}
		if (srcDir == Dir::BOTTOM && tgtDir == Dir::LEFT) {
			swapOffset = true;
		}

		return (swapOffset ? edgeLength - offset - 1 : offset);
	}

	Point calculatePosOnANewTile(const EndPoint& endPoint, int offset, const Dir& srcDir) {
		auto transformedOffset = claculateOffsetOnANewTile(endPoint, offset, srcDir);
		const auto& [tgtPos, tgtDir] = endPoint;
		if (tgtDir == Dir::TOP) {
			return { tgtPos.x * edgeLength + transformedOffset,  tgtPos.y * edgeLength };
		}
		if (tgtDir == Dir::BOTTOM) {
			return { tgtPos.x * edgeLength + transformedOffset,  (tgtPos.y + 1) * edgeLength - 1 };
		}
		if (tgtDir == Dir::LEFT) {
			return { tgtPos.x * edgeLength,  tgtPos.y * edgeLength + transformedOffset };
		}
		// tgtDir == Dir::RIGHT
		return { (tgtPos.x + 1) * edgeLength - 1 ,  tgtPos.y * edgeLength + transformedOffset };
	}

	bool advanceInCurrentDirOneStep() {
		Point currentTilePos{ currentPos.x / edgeLength, currentPos.y / edgeLength };
		EndPoint currentEndPoint{ currentTilePos, currentDir };
		auto target = findTarget(currentEndPoint);
		Dir newDir = currentDir;
		Point newPos{ currentPos.x + currentDir.x, currentPos.y + currentDir.y };

		if (currentDir.y != 0) {
			auto columnRange = columnRanges[newPos.x];
			if (newPos.y > columnRange.second || newPos.y < columnRange.first) {
				newDir = target->dir.opposite();
				auto xOffset = newPos.x - currentTilePos.x * edgeLength;
				newPos = calculatePosOnANewTile(target.value(), xOffset, currentEndPoint.dir);
			}
		}

		if (currentDir.x != 0) {
			const auto& row = map[newPos.y];
			const auto& rowOffset = rowOffsets[newPos.y];
			if (newPos.x < rowOffset || newPos.x >= row.size()) {
				newDir = target->dir.opposite();
				auto yOffset = newPos.y - currentTilePos.y * edgeLength;
				newPos = calculatePosOnANewTile(target.value(), yOffset, currentEndPoint.dir);
			}
		}
		
		if (map[newPos.y][newPos.x] == '#') {
			return false;
		}

		currentPos = newPos;
		currentDir = newDir;
		return true;
	}

	std::optional<EndPoint> findTarget(const EndPoint src) {
		auto found = std::find_if(connections.begin(), connections.end(), [src](auto&& connection) {
			return connection.src.pos == src.pos && connection.src.dir == src.dir;
		});
		if (found != connections.end()) {
			return found->tgt;
		}
		return std::nullopt;
	}

	const MapType& map;
	const std::vector<Connection>& connections;
	int edgeLength;
	Dir currentDir;
	std::vector<int> rowOffsets;
	std::vector<std::pair<int, int>> columnRanges;
	Point currentPos;
};

int dirToScore(const Dir& dir) {
	if (dir == Dir::RIGHT) {
		return 0;
	}
	if (dir == Dir::BOTTOM) {
		return 1;
	}
	if (dir == Dir::LEFT) {
		return 2;
	}
	return 3;
}

int calculateScore(const Point& finalPos, const Dir& finalDir) {
	auto finalRow = finalPos.y + 1;
	auto finalColumn = finalPos.x + 1;
	auto dirScore = dirToScore(finalDir);
	return 1000 * finalRow + 4 * finalColumn + dirScore;
}

int calculateEdgeLength(const MapType& map) {
	auto surfaceArea = std::accumulate(map.begin(), map.end(), 0, [](auto&& sum, auto&& row) {
		std::string charactersOnly = row;
		common::replaceAll(charactersOnly, " ", "");
		return sum + charactersOnly.size();
	});
	return sqrt(surfaceArea / 6);
}

GridType placeTilesOnGrid(const MapType& map, int edgeLength) {
	GridType grid(GRID_SIZE, std::vector<GridValue>(GRID_SIZE, GridValue::EMPTY));
	for (int y = 0; y < GRID_SIZE; ++y) {
		for (int x = 0; x < GRID_SIZE; ++x) {
			auto realY = y * edgeLength + edgeLength / 2;
			auto realX = x * edgeLength + edgeLength / 2;
			if (realY >= map.size()) {
				continue;
			}
			auto row = map[realY];
			if (realX >= row.size() || row[realX] == ' ') {
				continue;
			}
			grid[y][x] = GridValue::TILE;
		}
	}
	return grid;
}

std::vector<Connection> connectAdjacentLShaped(const GridType& grid, const Dir& neighborDir) {
	std::vector<Connection> connections;
	for (int y = 0; y < GRID_SIZE; ++y) {
		for (int x = 0; x < GRID_SIZE; ++x) {
			auto topLeft = grid[y][x];
			if (topLeft != GridValue::TILE) {
				continue;
			}

			Point targetPos{ x + neighborDir.x, y + neighborDir.y };
			if (targetPos.x < 0 || targetPos.x >= GRID_SIZE || targetPos.y < 0 || targetPos.y >= GRID_SIZE) {
				continue;
			}
			auto targetValue = grid[targetPos.y][targetPos.x];
			if (targetValue != GridValue::TILE) {
				continue;
			}

			bool isLshaped = grid[targetPos.y][x] == GridValue::TILE;
			if (isLshaped) {
				EndPoint src{ {x, y}, Dir{ neighborDir.x, 0} };
				EndPoint tgt{ targetPos, Dir::TOP };
				connections.push_back({ src, tgt });
			} else {
				EndPoint src{ {x, y}, Dir::BOTTOM };
				EndPoint tgt{ targetPos, Dir{ -neighborDir.x, 0} };
				connections.push_back({ src, tgt });
			}
		}
	}
	return connections;
}

std::vector<Connection> connectAdjacentToLShaped(const GridType& grid, const Connection& connection) {
	const auto& [src, tgt] = connection;
	std::vector<Connection> connections;
	auto srcAdjacent = common::tryGet(grid, src.pos.y, src.pos.x, tgt.dir);
	auto tgtAdjacent = common::tryGet(grid, tgt.pos.y, tgt.pos.x, src.dir);
	if (srcAdjacent.has_value() && srcAdjacent.value() == GridValue::TILE) {
		if (!tgtAdjacent.has_value() || tgtAdjacent.value() == GridValue::EMPTY) {
			EndPoint source{ {src.pos.x + tgt.dir.x, src.pos.y + tgt.dir.y}, src.dir };
			EndPoint target{ {tgt.pos.x, tgt.pos.y}, src.dir };
			connections.push_back({ source, target });
		}
	}
	if (tgtAdjacent.has_value() && tgtAdjacent.value() == GridValue::TILE) {
		if (!srcAdjacent.has_value() || srcAdjacent.value() == GridValue::EMPTY) {
			EndPoint source{ {src.pos.x, src.pos.y}, tgt.dir };
			EndPoint target{ {tgt.pos.x + src.dir.x, tgt.pos.y + src.dir.y}, tgt.dir };
			connections.push_back({ source, target });
		}
	}
	auto tgtAdjacentOpposite = common::tryGet(grid, tgt.pos.y, tgt.pos.x, tgt.dir.opposite());
	if (tgtAdjacentOpposite.has_value() && tgtAdjacentOpposite.value() == GridValue::TILE) {
		if (srcAdjacent.has_value() && srcAdjacent.value() == GridValue::TILE) {
			EndPoint source{ {src.pos.x + tgt.dir.x, src.pos.y + tgt.dir.y}, tgt.dir };
			EndPoint target{ {tgt.pos.x - tgt.dir.x, tgt.pos.y - tgt.dir.y}, src.dir };
			connections.push_back({ source, target });
		}
	}
	auto srcAdjacentOpposite = common::tryGet(grid, src.pos.y, src.pos.x, src.dir.opposite());
	if (srcAdjacentOpposite.has_value() && srcAdjacentOpposite.value() == GridValue::TILE) {
		if (tgtAdjacent.has_value() && tgtAdjacent.value() == GridValue::TILE) {
			EndPoint source{ {src.pos.x + src.dir.x, src.pos.y + src.dir.y}, tgt.dir };
			EndPoint target{ {tgt.pos.x - src.dir.x, tgt.pos.y - src.dir.y}, src.dir };
			connections.push_back({ source, target });
		}
	}
	return connections;
}

std::vector<Connection> connectVertical(const GridType& grid) {
	std::vector<Connection> connections;
	for (int x = 0; x < GRID_SIZE; ++x) {
		auto top = grid[0][x];
		auto bottomY = GRID_SIZE - 1;
		if (top == GridValue::TILE && top == grid[bottomY][x]) {
			EndPoint source{ {x, 0}, Dir::TOP };
			EndPoint target{ {x, bottomY}, Dir::BOTTOM };
			connections.push_back({ source, target });
		}
	}
	return connections;
}

std::vector<Connection> connectHorizontal(const GridType& grid) {
	std::vector<Connection> connections;
	for (int y = 0; y < GRID_SIZE; ++y) {
		auto left = grid[y].front();
		auto rightX = GRID_SIZE - 1;
		if (left == GridValue::TILE && left == grid[y].back()) {
			EndPoint source{ {0, y}, Dir::LEFT };
			EndPoint target{ {rightX, y}, Dir::RIGHT };
			connections.push_back({ source, target });
		}
	}
	return connections;
}

std::vector<EndPoint> generateAllPossibleendPoints(const GridType& grid) {
	std::vector<EndPoint> allPossibleEndPoints;
	for (int y = 0; y < GRID_SIZE; ++y) {
		for (int x = 0; x < GRID_SIZE; ++x) {
			if (grid[y][x] != GridValue::TILE) {
				continue;
			}
			auto possiblyAddEndPoint = [&allPossibleEndPoints, &grid](const Dir& dir, int x, int y) {
				auto neighbor = common::tryGet(grid, y, x, dir);
				if (!neighbor.has_value() || neighbor.value() == GridValue::EMPTY) {
					allPossibleEndPoints.push_back({ {x, y}, dir });
				}
			};
			possiblyAddEndPoint(Dir::TOP, x, y);
			possiblyAddEndPoint(Dir::LEFT, x, y);
			possiblyAddEndPoint(Dir::RIGHT, x, y);
			possiblyAddEndPoint(Dir::BOTTOM, x, y);
		}
	}
	return allPossibleEndPoints;
}

std::vector<EndPoint> findMissingEndPoints(const std::vector<EndPoint>& allPossibleEndPoints, const std::vector<Connection>& existingConnections) {
	std::vector<EndPoint> missingEndPoints;
	for (const auto& endPoint : allPossibleEndPoints) {
		auto found = std::find_if(existingConnections.begin(), existingConnections.end(), [endPoint](auto&& connection) {
			const bool isSrcSame = connection.src.pos == endPoint.pos && connection.src.dir == endPoint.dir;
			const bool isTgtSame = connection.tgt.pos == endPoint.pos && connection.tgt.dir == endPoint.dir;
			return isSrcSame || isTgtSame;
		});

		if (found == existingConnections.end()) {
			missingEndPoints.push_back(endPoint);
		}
	}
	return missingEndPoints;
}

std::vector<Connection> findMissingConnections(const GridType& grid, const std::vector<Connection>& connections) {
	auto allPossibleEndPoints = generateAllPossibleendPoints(grid);
	auto missingEndPoints = findMissingEndPoints(allPossibleEndPoints, connections);

	std::vector<Connection> missingConnections;
	if (missingEndPoints.size() > 0) {
		if (missingEndPoints.size() != 2) {
			std::cout << "This does not work" << std::endl;
			exit(-1);
		}
		missingConnections.push_back({ missingEndPoints[0], missingEndPoints[1] });
	}
	return missingConnections;
}

std::vector<Connection> connectCubeWalls(const MapType& map) {
	auto edgeLength = calculateEdgeLength(map);
	auto grid = placeTilesOnGrid(map, edgeLength);

	auto connections = connectAdjacentLShaped(grid, Dir::BOTOM_LEFT);
	auto toMerge = connectAdjacentLShaped(grid, Dir::BOTTOM_RIGHT);
	connections.insert(connections.end(), toMerge.begin(), toMerge.end());

	auto lShapedConnections = connections;
	for (const auto& connection : lShapedConnections) {
		auto newConnections = connectAdjacentToLShaped(grid, connection);
		connections.insert(connections.end(), newConnections.begin(), newConnections.end());
	}

	auto vertical = connectVertical(grid);
	connections.insert(connections.end(), vertical.begin(), vertical.end());

	auto horizotal = connectHorizontal(grid);
	connections.insert(connections.end(), horizotal.begin(), horizotal.end());

	auto missingConnections = findMissingConnections(grid, connections);
	connections.insert(connections.end(), missingConnections.begin(), missingConnections.end());

	auto toIterate = connections;
	for (const auto& [src, tgt] : toIterate) {
		connections.push_back({ tgt, src });
	}

	return connections;
}

int partOne(const DataType& data) {
	MapWalker walker(data.first);
	for (const auto& command : data.second) {
		walker.applyCommand(command);
	}
	return calculateScore(walker.getPosition(), walker.getDirection());
}

int partTwo(const DataType& data) {
	const auto& map = data.first;
	auto connections = connectCubeWalls(map);

	CubeWalker walker(data.first, connections, calculateEdgeLength(map));
	for (const auto& command : data.second) {
		walker.applyCommand(command);
	}
	return calculateScore(walker.getPosition(), walker.getDirection());
}

int main(int argc, char** argv) {
	const auto data = read();
	common::measureAndPrint("Part 1", partOne, data);
	common::measureAndPrint("Part 2", partTwo, data);
	return 0;
}