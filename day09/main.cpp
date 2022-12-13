#include "../common/pch.h"
#include <string>
#include <algorithm>
#include <vector>
#include <set>

struct Step {
	common::Dir dir;
	int moves;;
};

struct Pos {
	int x;
	int y;

	bool operator<(const Pos& right) const {
		if (x == right.x) {
			return y < right.y;
		}
		return x < right.x;
	}
};

using DataType = std::vector<Step>;
using common::Dir;

static const Pos STARTING_POINT{ 0, 0 };

Dir determineDir(char character) {
	if (character == 'U') {
		return Dir::TOP;
	}
	if (character == 'D') {
		return Dir::BOTTOM;
	}
	if (character == 'L') {
		return Dir::LEFT;
	}
	return Dir::RIGHT;
}

DataType read() {
	common::FileReader reader("input.txt");
	DataType data;
	for (std::string line; reader.nextLine(line) && !line.empty();) {
		Step step;
		step.dir = determineDir(line.front());
		step.moves = std::stoi(line.substr(2));
		data.push_back(step);
	}
	return data;
}

bool areInRange(const Pos& left, const Pos& right) {
	int xDiff = abs(left.x - right.x);
	int yDiff = abs(left.y - right.y);
	if (xDiff <= 1 && yDiff <= 1) {
		return true;
	}
	return false;
}

Pos getNewKnotPosition(const Pos& previousKnot, const Pos& knot) {
	if (areInRange(previousKnot, knot)) {
		return knot;
	}

	const int xDiff = previousKnot.x - knot.x;
	const int yDiff = previousKnot.y - knot.y;
	const int xUnit = xDiff != 0 ? abs(xDiff) / xDiff : 0;
	const int yUnit = yDiff != 0 ? abs(yDiff) / yDiff : 0;

	Pos newKnotPos = knot;
	while (!areInRange(newKnotPos, previousKnot)) {
		newKnotPos.x += xUnit;
		newKnotPos.y += yUnit;
	}
	return newKnotPos;
}

void moveKnots(std::vector<Pos>& knots, Dir dir) {
	auto& head = knots.front();
	head.x += dir.x;
	head.y += dir.y;

	for (int knotIdx = 1; knotIdx < knots.size(); ++knotIdx) {
		const auto& previous = knots[knotIdx - 1];
		auto& knot = knots[knotIdx];
		knot = getNewKnotPosition(previous, knot);
	}
}

int countTailVisits(const DataType& data, int numKnots) {
	std::vector<Pos> knots(numKnots, STARTING_POINT);

	auto& tail = knots.back();
	std::set<Pos> visitedByTail{ tail };

	for (const auto& step : data) {
		auto [dir, moves] = step;

		for (int i = 0; i < moves; ++i) {
			moveKnots(knots, dir);
			visitedByTail.insert({ tail.x, tail.y });
		}
	}

	return visitedByTail.size();
}

int partOne(const DataType& data) {
	return countTailVisits(data, 2);
}

int partTwo(const DataType& data) {
	return countTailVisits(data, 10);
}

int main(int argc, char** argv) {
	const auto data = read();
	common::measureAndPrint("Part 1", partOne, data);
	common::measureAndPrint("Part 2", partTwo, data);
	return 0;
}