#include "../common/pch.h"
#include <string>
#include <algorithm>
#include <numeric>
#include <vector>
#include <map>
#include <deque>
#include <functional>

class Monkey;
using InspectFunction = std::function<uint64_t(uint64_t)>;
using DataType = std::map<int, Monkey>;

class Item {
public:
	Item(uint64_t worryLevel) : worryLevel(worryLevel) {}

	void inspect(InspectFunction inspectFunction) {
		worryLevel = inspectFunction(worryLevel);
	}

	void inspectWithLcm(InspectFunction inspectFunction, uint64_t lcm) {
		worryLevel = inspectFunction(worryLevel);
		worryLevel %= lcm;
	}

	void divideWorryLevel() {
		worryLevel /= 3;
	}

	bool test(uint64_t divisor) const {
		return worryLevel % divisor == 0;
	}

private:
	uint64_t worryLevel;
};

struct ResultMap {
	int trueTestMonkeyId;
	int falseTestMoneyId;
};

class Monkey {
public:
	Monkey() {}
	Monkey(int id,
		InspectFunction inspectFunction,
		uint64_t divisor,
		ResultMap resultMap) :
		id(id),
		inspectFunction(inspectFunction),
		divisor(divisor),
		resultMap(resultMap) {
	}

	void addItem(const Item& item) {
		items.push_back(item);
	}

	void inspectAndThrow(DataType& monkeys) {
		while (!items.empty()) {
			auto item = items.front();
			items.pop_front();

			item.inspect(inspectFunction);
			item.divideWorryLevel();
			throwItem(monkeys, item);
			++numInspectedItems;
		}
	}

	void inspectAndThrowWithLcm(DataType& monkeys, uint64_t lcm) {
		while (!items.empty()) {
			auto item = items.front();
			items.pop_front();

			item.inspectWithLcm(inspectFunction, lcm);
			throwItem(monkeys, item);
			++numInspectedItems;
		}
	}

	uint64_t getNumInspectedItems() const {
		return numInspectedItems;
	}

	uint64_t getDivisor() const {
		return divisor;
	}

private:
	void throwItem(DataType& monkeys, const Item& item) {
		if (item.test(divisor)) {
			monkeys[resultMap.trueTestMonkeyId].addItem(item);
		} else {
			monkeys[resultMap.falseTestMoneyId].addItem(item);
		}
	}

	int id;
	std::deque<Item> items;
	InspectFunction inspectFunction;
	uint64_t divisor;
	ResultMap resultMap;
	uint64_t numInspectedItems{ 0 };
};

InspectFunction parseOperation(std::string line) {
	if (line.rfind("old") != line.find("old")) {
		return [](uint64_t worryLevel) {return worryLevel * worryLevel; };
	}

	char multiplicationPos = line.find('*');
	if (multiplicationPos != std::string::npos) {
		uint64_t value = std::stoi(line.substr(multiplicationPos + 2));
		return [value](uint64_t worryLevel) {return worryLevel * value; };
	}

	uint64_t value = std::stoi(line.substr(line.find('+') + 2));
	return [value](uint64_t worryLevel) {return worryLevel + value; };
}

uint64_t parseTest(std::string line) {
	line = line.substr(line.find("by") + 3);
	return std::stoi(line);
}

DataType read() {
	common::FileReader reader("input.txt");
	DataType data;
	for (std::string line; reader.nextLine(line);) {
		if (line.empty()) {
			continue;
		}

		const std::string onkey = "onkey";
		int monkeyId = std::stoi(line.substr(line.find(onkey) + onkey.size()));

		line = reader.nextLine();
		auto items = common::parseArray<uint64_t, Item>(line.substr(line.find(':') + 1), ',');

		line = reader.nextLine();
		InspectFunction inspectFunction = parseOperation(line);

		line = reader.nextLine();
		uint64_t divisor = parseTest(line);

		line = reader.nextLine();
		int trueId = std::stoi(line.substr(line.find(onkey) + onkey.size()));

		line = reader.nextLine();
		int falseId = std::stoi(line.substr(line.find(onkey) + onkey.size()));

		Monkey monkey(monkeyId, inspectFunction, divisor, {trueId, falseId});
		std::for_each(items.begin(), items.end(), [&monkey](const auto& item) {
			monkey.addItem(item);
		});
		data.insert({ monkeyId, monkey });
	}
	return data;
}

uint64_t calculateMonkeyActivity(const DataType& data) {
	std::vector<uint64_t> monkeyActivity;
	std::transform(data.begin(), data.end(), std::back_inserter(monkeyActivity), [](auto&& entry) {
		return entry.second.getNumInspectedItems();
		});

	std::sort(monkeyActivity.begin(), monkeyActivity.end(), std::greater<>());

	return monkeyActivity[0] * monkeyActivity[1];
}

uint64_t calculateLcm(const DataType& data) {
	std::vector<uint64_t> divisors;
	std::transform(data.begin(), data.end(), std::back_inserter(divisors), [](auto&& entry) {
		return entry.second.getDivisor();
	});

	uint64_t lcm = 1;
	for (auto divisor : divisors) {
		lcm = std::lcm(lcm, divisor);
	}
	
	return lcm;
}

uint64_t partOne(DataType data) {
	constexpr int numRounds = 20;

	for (int i = 0; i < numRounds; ++i) {
		for (auto& monkey : data) {
			monkey.second.inspectAndThrow(data);
		}
	}

	return calculateMonkeyActivity(data);
}

uint64_t partTwo(DataType data) {
	constexpr int numRounds = 10000;
	uint64_t lcm = calculateLcm(data);

	for (int i = 0; i < numRounds; ++i) {
		for (auto& monkey : data) {
			monkey.second.inspectAndThrowWithLcm(data, lcm);
		}
	}

	return calculateMonkeyActivity(data);
}

int main(int argc, char** argv) {
	const auto data = read();
	common::measureAndPrint("Part 1", partOne, data);
	common::measureAndPrint("Part 2", partTwo, data);
	return 0;
}