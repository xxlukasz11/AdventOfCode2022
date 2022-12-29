#include "../common/pch.h"
#include <cstdint>
#include <string>
#include <algorithm>
#include <vector>
#include <list>

using DataType = std::vector<int64_t>;

DataType read() {
	common::FileReader reader("input.txt");
	DataType data;
	for (std::string line; reader.nextLine(line);) {
		data.push_back(std::stoi(line));
	}
	return data;
}

int calculateTargetIndex(int64_t currentIndex, int64_t value, int64_t numElements) {
	auto target = currentIndex + value;
	if (target >= 0) {
		return target % numElements;
	}

	auto fullRanges = abs(target) / numElements;
	target += (fullRanges + 1)*numElements;
	return target;
}

int64_t sumKeys(const std::vector<int64_t>& values) {
	auto zeroIt = std::find(values.begin(), values.end(), 0);
	auto zeroPos = std::distance(values.begin(), zeroIt);

	const auto numElements = values.size();
	auto one = values[(zeroPos + 1000) % numElements];
	auto two = values[(zeroPos + 2000) % numElements];
	auto three = values[(zeroPos + 3000) % numElements];
	return one + two + three;
}

std::vector<int64_t> doMixing(DataType orderedNodes, int64_t decryptionKey, int numMixingReps) {
	std::list<DataType::iterator> mixingList;
	for (auto nodeIt = orderedNodes.begin(); nodeIt != orderedNodes.end(); ++nodeIt) {
		*nodeIt *= decryptionKey;
		mixingList.push_back(nodeIt);
	}

	const int numElements = mixingList.size();
	for (int mixingRepIdx = 0; mixingRepIdx < numMixingReps; ++mixingRepIdx) {

		for (auto nodeIt = orderedNodes.begin(); nodeIt != orderedNodes.end(); ++nodeIt) {
			if (*nodeIt == 0) {
				continue;
			}

			auto currentIt = std::find(mixingList.begin(), mixingList.end(), nodeIt);
			int currentIndex = std::distance(mixingList.begin(), currentIt);
			mixingList.erase(currentIt);

			int targetIndex = calculateTargetIndex(currentIndex, *nodeIt, numElements - 1);
			auto targetIt = std::next(mixingList.begin(), targetIndex);
			mixingList.insert(targetIt, nodeIt);
		}
	}

	std::vector<int64_t> numbers;
	std::transform(mixingList.begin(), mixingList.end(), std::back_inserter(numbers), [](auto&& node) {
		return *node;
	});
	return numbers;
}

int64_t partOne(const DataType& data) {
	constexpr int decryptionKey = 1;
	constexpr int numMixingReps = 1;
	auto values = doMixing(data, decryptionKey, numMixingReps);
	return sumKeys(values);
}

int64_t partTwo(const DataType& data) {
	constexpr int64_t decryptionKey = 811589153LL;
	constexpr int numMixingReps = 10;
	auto values = doMixing(data, decryptionKey, numMixingReps);
	return sumKeys(values);
}

int main(int argc, char** argv) {
	const auto data = read();
	common::measureAndPrint("Part 1", partOne, data);
	common::measureAndPrint("Part 2", partTwo, data);
	return 0;
}