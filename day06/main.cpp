#include "../common/pch.h"
#include <string>
#include <algorithm>
#include <vector>

using DataType = std::string;

DataType read() {
	common::FileReader reader("input.txt");
	return reader.nextLine();
}

int findPosOfUniqueCharacters(const DataType& data, int numOfCharacters) {
	for (auto markerStart = data.begin(); markerStart != data.end(); ++markerStart) {
		auto markerEnd = std::next(markerStart, numOfCharacters);
		std::vector<char> characters(markerStart, markerEnd);
		std::sort(characters.begin(), characters.end());
		auto sameValueFound = std::adjacent_find(characters.begin(), characters.end());
		if (sameValueFound == characters.end()) {
			return std::distance(data.begin(), markerEnd);
		}
	}
	return 0;
}

int partOne(const DataType& data) {
	return findPosOfUniqueCharacters(data, 4);
}

int partTwo(const DataType& data) {
	return findPosOfUniqueCharacters(data, 14);
}

int main(int argc, char** argv) {
	const auto data = read();
	common::measureAndPrint("Part 1", partOne, data);
	common::measureAndPrint("Part 2", partTwo, data);
	return 0;
}