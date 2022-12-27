#include "../common/pch.h"
#include <string>
#include <algorithm>
#include <numeric>
#include <vector>
#include <set>
#include <unordered_set>
#include <array>

using Cube = common::Vec3<int>;
using DataType = std::vector<Cube>;

DataType read() {
	common::FileReader reader("input.txt");
	DataType data;
	for (std::string line; reader.nextLine(line);) {
		auto cube = common::AggregateFactory<Cube>{','}.create<int, int, int>(line);
		data.push_back(cube);
	}
	return data;
}

std::array<Cube, 6> generateNeighbors(const Cube& cube) {
	return std::array<Cube, 6>{ {
			{cube.x + 1, cube.y, cube.z},
			{cube.x - 1, cube.y, cube.z },
			{cube.x, cube.y + 1, cube.z },
			{cube.x, cube.y - 1, cube.z },
			{cube.x, cube.y, cube.z + 1},
			{cube.x, cube.y, cube.z - 1}
		} };
}

bool isCubeInside(const Cube& cube, const Cube& minCube, const Cube& maxCube) {
	if (cube.x > maxCube.x || cube.x < minCube.x) {
		return false;
	}
	if (cube.y > maxCube.y || cube.y < minCube.y) {
		return false;
	}
	if (cube.z > maxCube.z || cube.z < minCube.z) {
		return false;
	}
	return true;
}

std::vector<Cube> generateValidNeighbors(const Cube& currentCube, const std::set<Cube>& outerCubes,
		const std::set<Cube>& cubes, const std::set<Cube>& currentLayer, const Cube& minCube, const Cube& maxCube) {
	auto all = generateNeighbors(currentCube);
	std::vector<Cube> valid;
	for (const auto& neighbor : all) {
		if (!isCubeInside(neighbor, minCube, maxCube)) {
			continue;
		}
		if (cubes.contains(neighbor)) {
			continue;
		}
		if (currentLayer.contains(neighbor)) {
			continue;
		}
		if (outerCubes.contains(neighbor)) {
			continue;
		}
		valid.push_back(neighbor);
	}
	return valid;
}

std::set<Cube> generateOuterCubes(const Cube& minCube, const Cube& maxCube) {
	std::set<Cube> cubes;
	for (int x = minCube.x; x <= maxCube.x; ++x) {
		for (int y = minCube.y; y <= maxCube.y; ++y) {
			cubes.insert({ x, y, minCube.z });
			cubes.insert({ x, y, maxCube.z });
		}
	}

	for (int x = minCube.x; x <= maxCube.x; ++x) {
		for (int z = minCube.z; z <= maxCube.z; ++z) {
			cubes.insert({ x, minCube.y, z });
			cubes.insert({ x, minCube.y, z });
		}
	}

	for (int z = minCube.z; z <= maxCube.z; ++z) {
		for (int y = minCube.y; y <= maxCube.y; ++y) {
			cubes.insert({ minCube.x, y, z });
			cubes.insert({ maxCube.x, y, z });
		}
	}

	return cubes;
}

std::set<Cube> getAllOuterCubes(const std::set<Cube>& cubes, const Cube& minCube, const Cube& maxCube) {
	std::set<Cube> outerCubes;
	auto currentLayer = generateOuterCubes(minCube, maxCube);
	while (!currentLayer.empty()) {
		std::set<Cube> nextLayer;
		for (const auto& cube : currentLayer) {
			auto neighbors = generateValidNeighbors(cube, outerCubes, cubes, currentLayer, minCube, maxCube);
			nextLayer.insert(neighbors.begin(), neighbors.end());
		}
		outerCubes.merge(currentLayer);
		currentLayer = std::move(nextLayer);
	}
	
	return outerCubes;
}

int partOne(const DataType& data) {
	std::unordered_set<Cube> cubes(data.begin(), data.end());
	int exposedSurfaceCount = 0;
	for (const auto& cube : cubes) {

		std::array<Cube, 6> neighbors = generateNeighbors(cube);
		for (const auto& neighbor : neighbors) {
			if (!cubes.contains(neighbor)) {
				++exposedSurfaceCount;
			}
		}
	}
	return exposedSurfaceCount;
}

int partTwo(const DataType& data) {
	const int minZ = std::min_element(data.begin(), data.end(), [](auto&& left, auto&& right) {
		return left.z < right.z;
	})->z;
	const int minY = std::min_element(data.begin(), data.end(), [](auto&& left, auto&& right) {
		return left.y < right.y;
	})->y;
	const int minX = std::min_element(data.begin(), data.end(), [](auto&& left, auto&& right) {
		return left.x < right.x;
	})->x;
	const int maxZ = std::max_element(data.begin(), data.end(), [](auto&& left, auto&& right) {
		return left.z < right.z;
	})->z;
	const int maxY = std::max_element(data.begin(), data.end(), [](auto&& left, auto&& right) {
		return left.y < right.y;
	})->y;
	const int maxX = std::max_element(data.begin(), data.end(), [](auto&& left, auto&& right) {
		return left.x < right.x;
	})->x;

	const Cube minCube{ minX - 1, minY - 1, minZ - 1 };
	const Cube maxCube{ maxX + 1, maxY + 1, maxZ + 1 };
	const std::set<Cube> cubes(data.begin(), data.end());

	const auto outerCubes = getAllOuterCubes(cubes, minCube, maxCube);
	
	int exposedSurfaceCount = 0;
	for (const auto& cube : outerCubes) {

		const auto neighbors = generateNeighbors(cube);
		for (const auto& neighbor : neighbors) {
			if (cubes.contains(neighbor)) {
				++exposedSurfaceCount;
			}
		}
	}
	return exposedSurfaceCount;
}

int main(int argc, char** argv) {
	const auto data = read();
	common::measureAndPrint("Part 1", partOne, data);
	common::measureAndPrint("Part 2", partTwo, data);
	return 0;
}