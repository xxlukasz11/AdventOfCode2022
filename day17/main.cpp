#include "../common/pch.h"
#include <cstdint>
#include <string>
#include <algorithm>
#include <numeric>
#include <vector>
#include <deque>
#include <array>

using Point = common::Point<int>;
using common::Dir;
using DataType = std::vector<Dir>;

constexpr char SYMBOL_STONE = '#';
constexpr int CHAMBER_WIDTH = 7;
constexpr std::array<char, CHAMBER_WIDTH> EMPTY_ROW{ {'.', '.', '.', '.', '.', '.', '.'} };
constexpr int INITIAL_POS_X = 2;
constexpr int NEW_BRICK_ROW_OFFSET = 3;

using MapType = std::deque<std::array<char, CHAMBER_WIDTH>>;
using ShapeType = std::vector<std::vector<char>>;

static const std::vector<ShapeType> shapes = {
	{
		{'#','#','#','#'}
	},
	{
		{'.','#','.'},
		{'#','#','#'},
		{'.','#','.'}
	},
	{
		{'.','.','#'},
		{'.','.','#'},
		{'#','#','#'}
	},
	{
		{'#'},
		{'#'},
		{'#'},
		{'#'}
	},
	{
		{'#','#'},
		{'#','#'}
	}
};

bool operator==(const Dir& left, const Dir& right) {
	return left.x == right.x && left.y == right.y;
}

DataType read() {
	common::FileReader reader("input.txt");
	DataType data;
	std::string line = reader.nextLine();
	std::transform(line.begin(), line.end(), std::back_inserter(data), [](auto&& character) {
		return character == '<' ? Dir::LEFT : Dir::RIGHT;
	});
	return data;
}

class Brick {
public:
	Brick(MapType& map, const ShapeType& shape) : map(map), shape(shape),
		shapeWidth(shape.front().size()), shapeHeight(shape.size()), topLeftPos(INITIAL_POS_X, 0) {}

	int getHeight() const {
		return shapeHeight;
	}

	int getTopPos() const {
		return topLeftPos.y;
	}

	void placeOnMap() const {
		for (int y = 0; y < shapeHeight; ++y) {
			for (int x = 0; x < shapeWidth; ++x) {
				auto& mapSymbol = map[topLeftPos.y + y][topLeftPos.x + x];
				if (mapSymbol != SYMBOL_STONE) {
					mapSymbol = shape[y][x];
				}
			}
		}
	}

	bool canBeMoved(const Dir& dir) const {
		if (dir == Dir::BOTTOM) {
			return checkBottom();
		}
		return checkSideMovement(dir);
	}

	void move(const Dir& dir) {
		topLeftPos.x += dir.x;
		topLeftPos.y += dir.y;
	}

	void updateTowerSizes(std::array<uint64_t, CHAMBER_WIDTH>& towerSizes, uint64_t mapBase) {
		for (int i = 0; i < shapeWidth; ++i) {
			int y = 0;
			while (shape[y][i] != SYMBOL_STONE) {
				++y;
			}
			auto relativeHeight = map.size() - (topLeftPos.y + y);
			auto realHeight = mapBase + relativeHeight;

			auto& towerValue = towerSizes[topLeftPos.x + i];
			towerValue = std::max(towerValue, realHeight);
		}
	}

private:
	bool checkBottom() const {
		for (int shapeRowIdx = shapeHeight - 1; shapeRowIdx >= 0; --shapeRowIdx) {
			int rowToBeMovedTo = topLeftPos.y + shapeRowIdx + 1;
			bool isRowMovementAllowed = checkRow(shapeRowIdx, rowToBeMovedTo);
			if (!isRowMovementAllowed) {
				return false;
			}
		}
		return true;
	}

	bool checkSideMovement(const Dir& dir) const {
		for (int shapeColumnIdx = 0; shapeColumnIdx < shapeWidth; ++shapeColumnIdx) {
			int columnToBeMovedTo = topLeftPos.x + shapeColumnIdx + dir.x;
			bool isColumnMovementAllowed = checkColumn(shapeColumnIdx, columnToBeMovedTo);
			if (!isColumnMovementAllowed) {
				return false;
			}
		}
		return true;
	}

	bool checkColumn(int shapeColumnIndex, int mapColumnIndex) const {
		if (mapColumnIndex >= CHAMBER_WIDTH || mapColumnIndex < 0) {
			return false;
		}

		for (int i = 0; i < shapeHeight; ++i) {
			if (shape[i][shapeColumnIndex] == SYMBOL_STONE && map[topLeftPos.y + i][mapColumnIndex] == SYMBOL_STONE) {
				return false;
			}
		}
		return true;
	}

	bool checkRow(int shapeRowIndex, int mapRowIndex) const {
		if (mapRowIndex >= map.size()) {
			return false;
		}

		const auto& shapeBottom = shape[shapeRowIndex];
		for (int i = 0; i < shapeWidth; ++i) {
			if (shapeBottom[i] == SYMBOL_STONE && map[mapRowIndex][topLeftPos.x + i] == SYMBOL_STONE) {
				return false;
			}
		}
		return true;
	}

	MapType& map;
	const ShapeType& shape;
	int shapeWidth;
	int shapeHeight;
	Point topLeftPos;
};

int partOne(const DataType& moveArray) {
	int towerHeight = 0;
	uint64_t brickIdx = 0;
	uint64_t moveIdx = 0;
	MapType map;
	for (int roundIdx = 0; roundIdx < 2022; ++roundIdx) {

		Brick brick{ map, shapes[brickIdx++ % shapes.size()] };
		int expectedMapSize = towerHeight + brick.getHeight() + NEW_BRICK_ROW_OFFSET;
		int missingSize = expectedMapSize - map.size();

		for (int i = 0; i < missingSize; ++i) {
			map.push_front(EMPTY_ROW);
		}
		for (int i = 0; i < -missingSize; ++i) {
			map.pop_front();
		}

		while (true) {
			auto jetDir = moveArray[moveIdx++ % moveArray.size()];
			if (brick.canBeMoved(jetDir)) {
				brick.move(jetDir);
			}

			if (brick.canBeMoved(Dir::BOTTOM)) {
				brick.move(Dir::BOTTOM);
				continue;
			}

			int brickTop = map.size() - brick.getTopPos();
			towerHeight = std::max(towerHeight, brickTop);
			brick.placeOnMap();
			break;
		}
	}

	return towerHeight;
}

uint64_t partTwo(const DataType& moveArray) {
	constexpr uint64_t rounds = 1000000000000;
	constexpr int bufferDepth = 400;
	constexpr int eraseThreshold = 200;

	uint64_t scoreBase = 0;
	uint64_t mapBaseHeight = 0;
	uint64_t maxTowerHeight = 0;
	uint64_t absoluteBrickIdx = 0;
	uint64_t absoluteMoveIdx = 0;
	uint64_t previousMoveIdx = 0;

	uint64_t previousMaxTowerHeight = 0;
	uint64_t previousMaxTowerDiff = 0;

	uint64_t previousRoundIdx = 0;
	uint64_t previousRoundDiff = 0;

	uint64_t previousMoveIdxAtWrap = 0;
	uint64_t previousMoveIdxDiff = 0;

	uint64_t previousBrickIdx = 0;
	uint64_t previousBrickIdxDiff = 0;

	bool moveArrayWrapped = false;
	bool roundsWereSkipped = false;

	MapType map;
	std::array<uint64_t, CHAMBER_WIDTH> towerSizes{ {0, 0, 0, 0, 0, 0, 0} };

	for (uint64_t roundIdx = 0; roundIdx < rounds; ++roundIdx) {
		if (moveArrayWrapped && !roundsWereSkipped) {
			const auto towerDiff = maxTowerHeight - previousMaxTowerHeight;
			const auto roundDiff = roundIdx - previousRoundIdx;
			const auto moveIdxDiff = absoluteMoveIdx - previousMoveIdxAtWrap;
			const auto brickIdxDiff = absoluteBrickIdx - previousBrickIdx;

			if (towerDiff == previousMaxTowerDiff && roundDiff == previousRoundDiff &&
					moveIdxDiff == previousMoveIdxDiff && brickIdxDiff == previousBrickIdxDiff) {
				const auto roundsLeft = rounds - roundIdx;
				const auto periods = roundsLeft / roundDiff;
				const auto roundsToSkip = periods * roundDiff;
				const auto scoreToAdd = periods * towerDiff;
				scoreBase += scoreToAdd;
				roundIdx += roundsToSkip;
				absoluteMoveIdx -= moveIdxDiff % moveArray.size();
				absoluteBrickIdx -= brickIdxDiff % shapes.size();
				roundsWereSkipped = true;
			}

			previousMaxTowerHeight = maxTowerHeight;
			previousMaxTowerDiff = towerDiff;

			previousRoundIdx = roundIdx;
			previousRoundDiff = roundDiff;

			previousMoveIdxAtWrap = absoluteMoveIdx;
			previousMoveIdxDiff = moveIdxDiff;

			previousBrickIdx = absoluteBrickIdx;
			previousBrickIdxDiff = brickIdxDiff;

			moveArrayWrapped = false;
		}

		const auto currentBrickIdx = absoluteBrickIdx++ % shapes.size();
		Brick brick{ map, shapes[currentBrickIdx] };
		int expectedMapSize = static_cast<int>(maxTowerHeight - mapBaseHeight) + brick.getHeight() + NEW_BRICK_ROW_OFFSET;
		int missingSize = expectedMapSize - map.size();

		for (int i = 0; i < missingSize; ++i) {
			map.push_front(EMPTY_ROW);
		}
		for (int i = 0; i < -missingSize; ++i) {
			map.pop_front();
		}

		while (true) {
			auto currentMoveIdx = absoluteMoveIdx++ % moveArray.size();
			if (!moveArrayWrapped && previousMoveIdx > currentMoveIdx) {
				moveArrayWrapped = true;
			}
			previousMoveIdx = currentMoveIdx;

			auto jetDir = moveArray[currentMoveIdx];
			if (brick.canBeMoved(jetDir)) {
				brick.move(jetDir);
			}

			if (brick.canBeMoved(Dir::BOTTOM)) {
				brick.move(Dir::BOTTOM);
				continue;
			}

			
			brick.placeOnMap();
			brick.updateTowerSizes(towerSizes, mapBaseHeight);
			maxTowerHeight = *std::max_element(towerSizes.begin(), towerSizes.end());
			uint64_t maxRowsToErase = *std::min_element(towerSizes.begin(), towerSizes.end()) - mapBaseHeight;
			
			if (map.size() > bufferDepth && map.size() - bufferDepth > eraseThreshold) {
				uint64_t rowsAllowedToErase = map.size() - bufferDepth;
				auto rowsToErase = std::min(maxRowsToErase, rowsAllowedToErase);
				mapBaseHeight += rowsToErase;
				map.resize(map.size() - rowsToErase);
			}
			break;
		}
	}

	return maxTowerHeight + scoreBase;
}

int main(int argc, char** argv) {
	const auto data = read();
	common::measureAndPrint("Part 1", partOne, data);
	common::measureAndPrint("Part 2", partTwo, data);
	return 0;
}