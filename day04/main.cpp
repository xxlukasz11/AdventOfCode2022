#include "../common/pch.h"
#include <string>
#include <sstream>
#include <vector>

struct Assignment {
	int min;
	int max;
};

struct Pair {
	Assignment left;
	Assignment right;
};

using DataType = std::vector<Pair>;

DataType read() {
	common::FileReader reader("input.txt");
	DataType data;
	for (std::string line; reader.nextLine(line);) {
		std::istringstream stream(line);
		Assignment left;
		stream >> left.min;
		stream.ignore();
		stream >> left.max;
		stream.ignore();

		Assignment right;
		stream >> right.min;
		stream.ignore();
		stream >> right.max;
		data.push_back({left, right});
	}
	return data;
}

bool fullyOverlap(const Pair& pair) {
	const auto& [left, right] = pair;
	if (left.min >= right.min && left.max <= right.max) {
		return true;
	}
	if (right.min >= left.min && right.max <= left.max) {
		return true;
	}
	return false;
}

bool partiallyOverlap(const Pair& pair) {
	const auto& [left, right] = pair;
	if (left.min <= right.max && left.max >= right.min) {
		return true;
	}
	if (right.min <= left.max && right.max >= left.min) {
		return true;
	}
	return false;
}

int partOne(const DataType& data) {
	int overlapCount = 0;
	for (const auto& pair : data) {
		overlapCount += fullyOverlap(pair);
	}
	return overlapCount;
}

int partTwo(const DataType& data) {
	int overlapCount = 0;
	for (const auto& pair : data) {
		overlapCount += partiallyOverlap(pair);
	}
	return overlapCount;
}

int main(int argc, char** argv) {
	const auto data = read();
	common::measureAndPrint("Part 1", partOne, data);
	common::measureAndPrint("Part 2", partTwo, data);
	return 0;
}