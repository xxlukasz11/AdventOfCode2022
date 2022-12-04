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

struct Rucksack {
	std::string left;
	std::string right;
};

using DataType = std::vector<Rucksack>;

DataType read() {
	common::FileReader reader("input.txt");
	DataType data;
	for (std::string line; reader.nextLine(line);) {
		Rucksack rucksack;
		rucksack.left = line.substr(0, line.size() / 2);
		rucksack.right = line.substr(line.size() / 2);
		data.push_back(rucksack);
	}
	return data;
}

bool isCharacterIn(char character, const std::string& compartment) {
	return std::any_of(compartment.begin(), compartment.end(), [character](auto&& c) {
		return character == c;
	});
}

std::string getCommonPart(const std::string& left, const std::string& right) {
	std::set<char> result;
	for (const auto& character : left) {
		if (isCharacterIn(character, right)) {
			result.insert(character);
		}
	}
	return std::string(result.begin(), result.end());
}

int getPriority(char character) {
	if (character >= 'a' && character <= 'z') {
		return character - 'a' + 1;
	}
	return character - 'A' + 27;
}

int getTotalPriority(const std::string& characters) {
	int priority = 0;
	for (auto&& character : characters) {
		priority += getPriority(character);
	}
	return priority;
}

int partOne(const DataType& data) {
	int score = 0;
	for (const auto& rucksack : data) {
		const auto& common = getCommonPart(rucksack.left, rucksack.right);
		score += getTotalPriority(common);
	}
	return score;
}

int partTwo(const DataType& data) {
	int score = 0;
	for (int i = 0; i < data.size(); i += 3) {
		const auto& one = data[i];
		const auto& two = data[i+1];
		const auto& three = data[i+2];
		const auto& commonOneTwo = getCommonPart(one.left + one.right, two.left + two.right);
		const auto& commonAll = getCommonPart(three.left + three.right, commonOneTwo);
		score += getTotalPriority(commonAll);
	}
	return score;
}

int main(int argc, char** argv) {
	const auto data = read();
	common::measureAndPrint("Part 1", partOne, data);
	common::measureAndPrint("Part 2", partTwo, data);
	return 0;
}