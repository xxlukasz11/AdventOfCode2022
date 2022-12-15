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

enum class Instruction {
	ADDX,
	NOOP
};

struct Operation {
	Instruction instruction;
	int value;
};

constexpr int ADDX_CYCLES = 2;
using DataType = std::vector<Operation>;

DataType read() {
	common::FileReader reader("input.txt");
	DataType data;
	for (std::string line; reader.nextLine(line);) {
		Operation operation;
		if (line.starts_with("addx")) {
			operation.instruction = Instruction::ADDX;
			operation.value = std::stoi(line.substr(5));
		} else {
			operation.instruction = Instruction::NOOP;
		}
		data.push_back(operation);
	}
	return data;
}

class Engine {
public:
	Engine(const DataType& operations) :
		operations(operations),
		currentOp(operations.begin()) {
	}

	void nextCycle() {
		if (!executionBlocked) {
			if (currentCycle != 1) {
				++currentOp;
			}
			if (currentOp->instruction == Instruction::ADDX) {
				lockedUntilCycleEnd = currentCycle + ADDX_CYCLES - 1;
				executionBlocked = true;
			}
		}

		if (lockedUntilCycleEnd == currentCycle) {
			registerValue += currentOp->value;
			executionBlocked = false;
		}
		++currentCycle;
	}

	int getCurrentCycle() const {
		return currentCycle;
	}

	int getRegisterValue() const {
		return registerValue;
	}

private:
	DataType operations;
	DataType::const_iterator currentOp;
	int currentCycle{ 1 };
	int registerValue{ 1 };
	int lockedUntilCycleEnd{ 0 };
	bool executionBlocked{ false };
};

int partOne(const DataType& data) {
	std::vector<int> cyclesToCheck{220, 180, 140, 100, 60, 20};
	int signalStrength = 0;

	Engine engine(data);
	while (!cyclesToCheck.empty()) {
		int cycleIdx = engine.getCurrentCycle();
		if (cyclesToCheck.back() == cycleIdx) {
			cyclesToCheck.pop_back();
			signalStrength += engine.getRegisterValue() * cycleIdx;
		}
		engine.nextCycle();
	}
	return signalStrength;
}

std::string partTwo(const DataType& data) {
	constexpr int crtWidth = 40;
	constexpr int crtHeight = 6;

	std::vector<std::vector<char>> crt;
	crt.reserve(crtWidth * crtHeight);
	int pixelsDrawn = 0;

	Engine engine(data);
	while (pixelsDrawn < crtWidth * crtHeight) {
		if (crt.size() * crtWidth == pixelsDrawn) {
			crt.push_back({});
		}

		auto& row = crt.back();
		const int pixelPos = row.size();
		const int spritePos = engine.getRegisterValue();
		if (pixelPos >= spritePos - 1 && pixelPos <= spritePos + 1) {
			row.push_back('#');
		} else {
			row.push_back('.');
		}
		++pixelsDrawn;

		engine.nextCycle();
	}

	for (const auto& row : crt) {
		for (const auto& pixel : row) {
			std::cout << pixel;
		}
		std::cout << std::endl;
	}

	return "EJCFPGLH";
}

int main(int argc, char** argv) {
	const auto data = read();
	common::measureAndPrint("Part 1", partOne, data);
	common::measureAndPrint("Part 2", partTwo, data);
	return 0;
}