#include "../common/pch.h"
#include <string>
#include <vector>
#include <set>

constexpr char START_MARKER = 'S';
constexpr char END_MARKER = 'E';
constexpr char START_ELEWATION = 'a';
constexpr char END_ELEWATION = 'z';

using DataType = std::vector<std::vector<char>>;

struct Pos {
	int row;
	int col;
};

struct Node {
	int row;
	int col;
	int distance;
	char elevation;
};

bool operator<(const Node& n1, const Node& n2) {
	if (n1.distance == n2.distance) {
		return n1.col != n2.col ? n1.col < n2.col : n1.row < n2.row;
	}
	return n1.distance < n2.distance;
}

int dijkstraAlgorithm(const DataType& data, const Pos& start, const Pos& end) {
	const int rows = data.size();
	const int cols = data[0].size();
	const int maxDistance = cols * rows * 10;

	auto distances = std::vector<std::vector<int>>(rows, std::vector<int>(cols, maxDistance));
	std::set<Node> nodes;
	nodes.insert(Node{ start.row, start.col, 0, START_ELEWATION });
	distances[start.row][start.col] = 0;

	while (!nodes.empty()) {
		auto node = *nodes.begin();
		nodes.erase(nodes.begin());
		auto allNeighbors = common::findNeighbors(data, node.row, node.col, 1, common::SearchPolicy::ADJACENT);
		for (const auto& neighbor : allNeighbors) {
			char neighborElewation = neighbor.value;
			if (node.elevation < neighborElewation - 1 && neighborElewation != END_MARKER) {
				continue;
			}

			if (neighborElewation == END_MARKER && node.elevation != END_ELEWATION) {
				continue;
			}

			int nR = neighbor.row;
			int nC = neighbor.col;
			int nodeDistance = distances[node.row][node.col] + 1;
			if (distances[nR][nC] > nodeDistance) {
				auto found = nodes.find(Node{ nR, nC, distances[nR][nC] });
				if (found != nodes.end()) {
					nodes.erase(found);
				}
				nodes.insert(Node{ nR, nC, nodeDistance, neighborElewation });
				distances[nR][nC] = nodeDistance;
			}
		}
	}

	return distances[end.row][end.col];
}

DataType read() {
	common::FileReader reader("input.txt");
	DataType data;
	for (std::string line; reader.nextLine(line);) {
		std::vector<char> row(line.begin(), line.end());
		data.push_back(row);
	}
	return data;
}

Pos findPos(const DataType& data, char character) {
	const int rows = data.size();
	const int cols = data.front().size();
	for (int row = 0; row < rows; ++row) {
		for (int col = 0; col < cols; ++col) {
			if (data[row][col] == character) {
				return { row, col };
			}
		}
	}
	return { 0 , 0 };
}

std::vector<Pos> findAllPos(const DataType& data, char character) {
	const int rows = data.size();
	const int cols = data.front().size();
	std::vector<Pos> matching;
	for (int row = 0; row < rows; ++row) {
		for (int col = 0; col < cols; ++col) {
			if (data[row][col] == character) {
				matching.push_back({ row, col });
			}
		}
	}
	return matching;
}

int partOne(const DataType& data) {
	auto start = findPos(data, START_MARKER);
	auto end = findPos(data, END_MARKER);
	return dijkstraAlgorithm(data, start, end);
}

int partTwo(const DataType& data) {
	auto startPoints = findAllPos(data, START_ELEWATION);
	auto end = findPos(data, END_MARKER);

	int shortestPath = 10000000;
	for (const auto& start : startPoints) {
		auto pathLength = dijkstraAlgorithm(data, start, end);
		if (pathLength < shortestPath) {
			shortestPath = pathLength;
		}
	}
	return shortestPath;
}

int main(int argc, char** argv) {
	const auto data = read();
	common::measureAndPrint("Part 1", partOne, data);
	common::measureAndPrint("Part 2", partTwo, data);
	return 0;
}