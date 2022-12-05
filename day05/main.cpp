#include "../common/pch.h"
#include <string>
#include <algorithm>
#include <sstream>
#include <vector>

struct Move {
	int quantity;
	int src;
	int tgt;
};

using Stack = std::vector<char>;
struct Input {
	std::vector<Stack> stacks;
	std::vector<Move> moves;
};

using DataType = Input;

void parseStackRow(std::vector<Stack>& stacks, const std::string& line) {
	constexpr int charactersPerStack = 4;
	for (int i = 0; i < line.size(); i += charactersPerStack) {
		int stackIndex = i / charactersPerStack;
		if (stacks.size() == stackIndex) {
			stacks.push_back({});
		}
		if (line[i] == '[') {
			stacks[stackIndex].push_back(line[i + 1]);
		}
	}
}

Move parseMove(std::string line) {
	common::replaceFirst(line, "move", "");
	common::replaceFirst(line, "from", "");
	common::replaceFirst(line, "to", "");
	std::istringstream stream(line);
	Move move;
	stream >> move.quantity >> move.src >> move.tgt;
	return move;
}

DataType read() {
	common::FileReader reader("input.txt");
	DataType data;
	for (std::string line; reader.nextLine(line);) {
		if (line.empty()) {
			break;
		}
		if (line.find('[') == std::string::npos) {
			continue;
		}
		parseStackRow(data.stacks, line);
	}
	for (auto& stack : data.stacks) {
		std::reverse(stack.begin(), stack.end());
	}
	for (std::string line; reader.nextLine(line);) {
		data.moves.push_back(parseMove(line));
	}
	return data;
}

std::string getStackTopElements(const std::vector<Stack>& stacks) {
	std::string result;
	for (const auto& stack : stacks) {
		if (!stack.empty()) {
			result += stack.back();
		}
	}
	return result;
}

std::string partOne(DataType data) {
	auto& [stacks, moves] = data;
	for (const auto& move : moves) {
		auto& srcStack = stacks[move.src - 1];
		auto& tgtStack = stacks[move.tgt - 1];
		auto splitPoint = std::next(srcStack.begin(), srcStack.size() - move.quantity);
		std::reverse_copy(splitPoint, srcStack.end(), std::back_inserter(tgtStack));
		srcStack.erase(splitPoint, srcStack.end());
	}
	return getStackTopElements(stacks);
}

std::string partTwo(DataType data) {
	auto& [stacks, moves] = data;
	for (const auto& move : moves) {
		auto& srcStack = stacks[move.src - 1];
		auto& tgtStack = stacks[move.tgt - 1];
		auto splitPoint = std::next(srcStack.begin(), srcStack.size() - move.quantity);
		std::copy(splitPoint, srcStack.end(), std::back_inserter(tgtStack));
		srcStack.erase(splitPoint, srcStack.end());
	}
	return getStackTopElements(stacks);
}

int main(int argc, char** argv) {
	const auto data = read();
	common::measureAndPrint("Part 1", partOne, data);
	common::measureAndPrint("Part 2", partTwo, data);
	return 0;
}