#include "../common/pch.h"
#include <cstdint>
#include <string>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <regex>

enum class Action {
	ADD,
	SUBTRACT,
	DIVIDE,
	MULTIPLY
};

struct Operation {
	std::string left;
	std::string right;
	Action action{ Action::ADD };
};

struct Monkey {
	bool isYellingValue{ false };
	std::string name;
	int64_t value{ 0 };
	Operation operation;
};

using DataType = std::unordered_map <std::string, Monkey>;

static const std::string ROOT_NAME = "root";
static const std::string HUMAN_NAME = "humn";

Action strToAction(const std::string& str) {
	if (str == "*") {
		return Action::MULTIPLY;
	}
	if (str == "-") {
		return Action::SUBTRACT;
	}
	if (str == "/") {
		return Action::DIVIDE;
	}
	return Action::ADD;
}

DataType read() {
	common::FileReader reader("input.txt");
	DataType data;
	for (std::string line; reader.nextLine(line);) {
		std::regex valuePattern("(\\w+):\\s(\\d+)");
		std::smatch match;
		if (std::regex_match(line, match, valuePattern)) {
			Monkey monkey;
			monkey.isYellingValue = true;
			monkey.name = match[1];
			monkey.value = std::stoi(match[2]);
			data.insert({ monkey.name, monkey });
			continue;
		}

		std::regex actionPattern("(\\w+):\\s(\\w+)\\s([+-/*])\\s(\\w+)");
		if(std::regex_match(line, match, actionPattern)) {
			Monkey monkey;
			monkey.name = match[1];
			monkey.operation = Operation{ match[2], match[4], strToAction(match[3]) };
			data.insert({ monkey.name, monkey });
		}
	}
	return data;
}

int64_t calculate(int64_t left, int64_t right, Action action) {
	switch (action) {
		case Action::ADD:
			return left + right;
		case Action::SUBTRACT:
			return left - right;
		case Action::DIVIDE:
			return left / right;
		case Action::MULTIPLY:
			return left * right;
	}
	return 0;
}

int64_t reverseCalculation(int64_t input, int64_t result, Action action, bool isRightATarget) {
	if (Action::ADD == action) {
		return result - input;
	}

	if (Action::MULTIPLY == action) {
		return result / input;
	}

	if (Action::DIVIDE == action && isRightATarget) {
		return input / result;
	}

	if (Action::DIVIDE == action && !isRightATarget) {
		return input * result;
	}

	if (Action::SUBTRACT == action && isRightATarget) {
		return input - result;
	}

	// subtract && !isRightATarget
	return input + result;
}

int64_t resolveValue(
		const std::unordered_map<std::string, Monkey>& monkeys,
		const std::string& monkeyName,
		std::unordered_map<std::string, int64_t>& results) {
	auto resultFound = results.find(monkeyName);
	if (resultFound != results.end()) {
		return resultFound->second;
	}

	const auto& monkey = monkeys.find(monkeyName)->second;
	if (monkey.isYellingValue) {
		results.insert({monkeyName, monkey.value});
		return monkey.value;
	}

	const auto& operation = monkey.operation;
	const auto left = resolveValue(monkeys, operation.left, results);
	const auto right = resolveValue(monkeys, operation.right, results);
	const auto result = calculate(left, right, operation.action);

	results.insert({ monkeyName, result });
	return result;
}

std::unordered_map<std::string, int64_t> getResultMap(const DataType& monkeys) {
	std::unordered_map<std::string, int64_t> results;
	resolveValue(monkeys, ROOT_NAME, results);
	return results;
}

std::vector<std::string> createPathToHuman(const DataType& monkeys) {
	std::vector<std::string> pathToHuman;
	std::string currentName = HUMAN_NAME;
	while (currentName != ROOT_NAME) {
		pathToHuman.push_back(currentName);
		auto parent = std::find_if(monkeys.begin(), monkeys.end(), [&currentName](auto&& entry) {
			const auto& monkey = entry.second;
			return monkey.operation.left == currentName || monkey.operation.right == currentName;
		});
		currentName = parent->second.name;
	}
	return pathToHuman;
}

int64_t partOne(const DataType& monkeys) {
	auto results = getResultMap(monkeys);
	return results[ROOT_NAME];
}

int64_t partTwo(const DataType& monkeys) {
	Monkey currentMonkey = monkeys.find(ROOT_NAME)->second;
	int64_t targetMonkeyExpectedResult = 0;
	const auto results = getResultMap(monkeys);
	auto pathToHuman = createPathToHuman(monkeys);

	while (currentMonkey.name != HUMAN_NAME) {
		auto targetMonkeyName = pathToHuman.back();
		pathToHuman.pop_back();

		const auto& leftMonkeyName = currentMonkey.operation.left;
		const auto& rightMonkeyName = currentMonkey.operation.right;
		const bool isRightMonkeyATarget = rightMonkeyName == targetMonkeyName;
		const auto& otherMonkeyName = isRightMonkeyATarget ? leftMonkeyName : rightMonkeyName;
		const auto otherMonkeyResult = results.find(otherMonkeyName)->second;

		if (currentMonkey.name == ROOT_NAME) {
			targetMonkeyExpectedResult = otherMonkeyResult;
		} else {
			targetMonkeyExpectedResult = reverseCalculation(otherMonkeyResult,
				targetMonkeyExpectedResult, currentMonkey.operation.action, isRightMonkeyATarget);
		}
		currentMonkey = monkeys.find(targetMonkeyName)->second;
	}

	return targetMonkeyExpectedResult;
}

int main(int argc, char** argv) {
	const auto data = read();
	common::measureAndPrint("Part 1", partOne, data);
	common::measureAndPrint("Part 2", partTwo, data);
	return 0;
}