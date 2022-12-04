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

struct Elf {
	std::vector<int> calories;
};

using DataType = std::vector<Elf>;

DataType read() {
	common::FileReader reader("input.txt");
	DataType data;
	Elf elf;
	for (std::string line; reader.nextLine(line);) {
		if (line.empty()) {
			data.push_back(elf);
			elf = Elf{};
			continue;
		}
		auto value = std::stoi(line);
		elf.calories.push_back(value);
	}
	return data;
}

int partOne(const DataType& data) {
	int maxCalories = 0;
	for (const auto& elf : data) {
		auto calories = std::accumulate(elf.calories.begin(), elf.calories.end(), 0);
		maxCalories = std::max(calories, maxCalories);
	}
	return maxCalories;
}

int partTwo(const DataType& data) {
	std::set<int> totalCalories;
	for (const auto& elf : data) {
		auto calories = std::accumulate(elf.calories.begin(), elf.calories.end(), 0);
		totalCalories.insert(calories);
	}
	auto biggest = totalCalories.rbegin();
	auto third = std::next(biggest, 3);
	return std::accumulate(biggest, third, 0);
}

int main(int argc, char** argv) {
	const auto data = read();
	common::measureAndPrint("Part 1", partOne, data);
	common::measureAndPrint("Part 2", partTwo, data);
	return 0;
}