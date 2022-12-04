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

enum Turn {
	ROCK = 1,
	PAPER,
	SCISSORS
};

struct Round {
	Turn me;
	Turn opponent;
};

static const std::unordered_map<char, Turn> letterMapping{
	{'A', ROCK},
	{'B', PAPER},
	{'C', SCISSORS},
	{'X', ROCK},
	{'Y', PAPER},
	{'Z', SCISSORS},
};

using DataType = std::vector<Round>;

DataType read() {
	common::FileReader reader("input.txt");
	DataType data;
	for (std::string line; reader.nextLine(line);) {
		auto letters = common::parseArray<char>(line);
		data.push_back({ letterMapping.find(letters[1])->second, letterMapping.find(letters[0])->second });
	}
	return data;
}

Turn getWinning(Turn turn) {
	if (turn == ROCK) {
		return PAPER;
	}
	if (turn == PAPER) {
		return SCISSORS;
	}
	return ROCK;
}

Turn getLosing(Turn turn) {
	if (turn == ROCK) {
		return SCISSORS;
	}
	if (turn == PAPER) {
		return ROCK;
	}
	return PAPER;
}

int calculateMyPoints(const Round& round) {
	auto [me, op] = round;
	if (me == op) {
		return 3 + me;
	}
	if (me == getWinning(op)) {
		return 6 + me;
	}
	return me;
}

int calculateMyPointsPart2(const Round& round) {
	enum Result {
		LOSE = ROCK,
		DRAW,
		WIN,
	};

	auto [me, op] = round;
	Round newRound = round;
	if (me == DRAW) {
		newRound.me = op;
	} else if (me == LOSE) {
		newRound.me = getLosing(op);
	} else {
		newRound.me = getWinning(op);
	}
	return calculateMyPoints(newRound);
}

int partOne(const DataType& data) {
	int score = 0;
	for (const auto& round : data) {
		score += calculateMyPoints(round);
	}
	return score;
}

int partTwo(const DataType& data) {
	int score = 0;
	for (const auto& round : data) {
		score += calculateMyPointsPart2(round);
	}
	return score;
}

int main(int argc, char** argv) {
	const auto data = read();
	common::measureAndPrint("Part 1", partOne, data);
	common::measureAndPrint("Part 2", partTwo, data);
	return 0;
}