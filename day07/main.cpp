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

constexpr int COMMAND_POS = 2;
constexpr int COMMAND_LENGTH = 2;
constexpr int COMMAND_ARG_POS = 5;
constexpr int DIR_NAME_POS = 4;

enum class EntryType {
	FILE,
	DIR
};

struct Entry;

struct Directory {
	std::optional<int> size;
	std::list<std::weak_ptr<Entry>> entries;
};

struct File {
	int size{ 0 };
};

struct Entry {
	Entry(EntryType type, const std::string& name) :
		type(type),
		name(name) {
	}
	~Entry() {}

	EntryType type;
	std::string name;
	std::weak_ptr<Entry> parent;

	Directory directory;
	File file;

	int getSize() {
		if (type == EntryType::FILE) {
			return file.size;
		}
		if (directory.size.has_value()) {
			return directory.size.value();
		}
		int size = 0;
		for (const auto& entry : directory.entries) {
			size += entry.lock()->getSize();
		}
		directory.size = size;
		return size;
	}

	std::optional<std::weak_ptr<Entry>> getEntry(const std::string& entryName) {
		auto found = std::find_if(directory.entries.begin(), directory.entries.end(), [&entryName](auto&& entry) {
			return entry.lock()->name == entryName;
		});
		if (found == directory.entries.end()) {
			return std::nullopt;
		}
		return *found;
	}
};

using DataType = std::list<std::shared_ptr<Entry>>;

DataType read() {
	common::FileReader reader("input.txt");
	DataType data;
	std::weak_ptr<Entry> currentEntry;
	std::weak_ptr<Entry> root;
	for (std::string line; reader.nextLine(line);) {
		if (line.starts_with("$")) {
			std::string command = line.substr(COMMAND_POS, COMMAND_LENGTH);
			if (command == "cd") {
				std::string path = line.substr(COMMAND_ARG_POS);
				if (currentEntry.expired()) {
					auto entry = std::make_shared<Entry>(EntryType::DIR, path);
					currentEntry = entry;
					data.push_back(entry);
					root = entry;
				}
				else if (path == "..") {
					currentEntry = currentEntry.lock()->parent;
				}
				else if (path == "/") {
					currentEntry = root;
				}
				else {
					auto entryOpt = currentEntry.lock()->getEntry(path);
					if (entryOpt.has_value()) {
						currentEntry = entryOpt.value();
					}
					else {
						auto entry = std::make_shared<Entry>(EntryType::DIR, path);
						entry->parent = currentEntry;
						currentEntry.lock()->directory.entries.push_back(entry);
						currentEntry = entry;
						data.push_back(entry);
					}
				}
			}
		}
		else if (line.starts_with("dir")) {
			std::string dirName = line.substr(DIR_NAME_POS);
			auto entry = std::make_shared<Entry>(EntryType::DIR, dirName);
			entry->parent = currentEntry;
			currentEntry.lock()->directory.entries.push_back(entry);
			data.push_back(entry);
		}
		else {
			std::istringstream stream(line);
			int fileSize;
			std::string fileName;
			stream >> fileSize >> fileName;
			auto entry = std::make_shared<Entry>(EntryType::FILE, fileName);
			entry->parent = currentEntry;
			entry->file.size = fileSize;
			currentEntry.lock()->directory.entries.push_back(entry);
			data.push_back(entry);
		}
	}
	return data;
}

int partOne(const DataType& data) {
	int totalDirSize = 0;
	for (auto&& entry : data) {
		if (entry->type != EntryType::DIR) {
			continue;
		}
		auto dirSize = entry->getSize();
		if (dirSize <= 100000) {
			totalDirSize += dirSize;
		}
	}
	return totalDirSize;
}

int partTwo(const DataType& data) {
	DataType directories;
	std::copy_if(data.begin(), data.end(), std::back_inserter(directories), [](auto&& entry) {
		return entry->type == EntryType::DIR;
	});

	directories.sort([](auto&& left, auto&& right) {
		return left->getSize() < right->getSize();
	});

	constexpr auto availableSpace = 70000000;
	constexpr auto wantedSpace = 30000000;
	const auto usedSpace = directories.back()->getSize();
	const auto freeSpace = availableSpace - usedSpace;
	const auto missingSpace = wantedSpace - freeSpace;

	for (auto&& dir : directories) {
		const auto dirSize = dir->getSize();
		if (dirSize >= missingSpace) {
			return dirSize;
		}
	}
	return 0;
}

int main(int argc, char** argv) {
	const auto data = read();
	common::measureAndPrint("Part 1", partOne, data);
	common::measureAndPrint("Part 2", partTwo, data);
	return 0;
}