#include "../common/pch.h"
#include <string>
#include <algorithm>
#include <utility>
#include <vector>
#include <set>

using DataType = std::vector<std::vector<int>>;

DataType read() {
	common::FileReader reader("input.txt");
	DataType data;
	for (std::string line; reader.nextLine(line);) {
		std::vector<int> digits;
		for (auto character : line) {
			digits.push_back(character - '0');
		}
		data.push_back(digits);
	}
	return data;
}

template<typename Container>
class MatrixIterator {
public:
	using ValueType = typename Container::value_type::value_type;

	MatrixIterator(const Container& matrix, int row, int col, common::Dir dir) : matrix(matrix), row(row), col(col), dir(dir) {}

	bool hasValue() {
		auto currentValue = common::tryGet(matrix, row, col);
		return currentValue.has_value();
	}

	int getCol() const {
		return col;
	}

	int getRow() const {
		return row;
	}

	ValueType next() {
		auto currentValue = common::tryGet(matrix, row, col);
		row += dir.y;
		col += dir.x;
		return currentValue.value();
	}

private:
	const Container& matrix;
	int row;
	int col;
	common::Dir dir;
};

std::vector<std::pair<int, int>> getVisibleTrees(const DataType& data, int rowIdx, int colIdx, common::Dir dir) {
	std::vector<std::pair<int, int>> visibleTrees;
	MatrixIterator it(data, rowIdx, colIdx, dir);
	for (int highestTree = -1; it.hasValue();) {
		auto col = it.getCol();
		auto row = it.getRow();
		auto nextTree = it.next();
		if (nextTree <= highestTree) {
			continue;
		}
		highestTree = std::max(nextTree, highestTree);
		visibleTrees.emplace_back(row, col);
	}
	return visibleTrees;
}

int partOne(const DataType& data) {
	std::set<std::pair<int, int>> visibleTrees;
	const auto rows = data.size();
	const auto cols = data.back().size();

	for (int rowIdx = 0; rowIdx < rows; ++rowIdx) {
		auto treesFromTheLeftSide = getVisibleTrees(data, rowIdx, 0, common::Dir::RIGHT);
		visibleTrees.insert(treesFromTheLeftSide.begin(), treesFromTheLeftSide.end());

		auto treesFromTheRightSide = getVisibleTrees(data, rowIdx, cols-1, common::Dir::LEFT);
		visibleTrees.insert(treesFromTheRightSide.begin(), treesFromTheRightSide.end());
	}

	for (int colIdx = 0; colIdx < cols; ++colIdx) {
		auto treesFromTheUpperSide = getVisibleTrees(data, 0, colIdx, common::Dir::BOTTOM);
		visibleTrees.insert(treesFromTheUpperSide.begin(), treesFromTheUpperSide.end());

		auto treesFromTheBottomSide = getVisibleTrees(data, rows-1, colIdx, common::Dir::TOP);
		visibleTrees.insert(treesFromTheBottomSide.begin(), treesFromTheBottomSide.end());
	}
	
	return visibleTrees.size();
}

int calculateViewingDistance(const DataType& data, int row, int col, common::Dir dir) {
	int houseTree = data[row][col];
	int visibleTrees = 0;
	MatrixIterator it(data, row, col, dir);
	it.next();
	while (it.hasValue()) {
		++visibleTrees;
		auto nextTree = it.next();
		if (nextTree >= houseTree) {
			break;
		}
	}
	return visibleTrees;
}

int partTwo(const DataType& data) {
	const auto rows = data.size();
	const auto cols = data.back().size();
	int maxScore = 0;
	for (int row = 1; row < rows-1; ++row) {
		for (int col = 1; col < cols-1; ++col) {
			int score = 1;
			score *= calculateViewingDistance(data, row, col, common::Dir::TOP);
			score *= calculateViewingDistance(data, row, col, common::Dir::BOTTOM);
			score *= calculateViewingDistance(data, row, col, common::Dir::RIGHT);
			score *= calculateViewingDistance(data, row, col, common::Dir::LEFT);
			maxScore = std::max(maxScore, score);
		}
	}
	return maxScore;
}

int main(int argc, char** argv) {
	const auto data = read();
	common::measureAndPrint("Part 1", partOne, data);
	common::measureAndPrint("Part 2", partTwo, data);
	return 0;
}