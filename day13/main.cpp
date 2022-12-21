#include "../common/pch.h"
#include <string>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <vector>

int intCompare(int left, int right) {
	if (left < right) {
		return 1;
	}
	if (left > right) {
		return -1;
	}
	return 0;
}

struct Element {
	static Element createValueElement(int value) {
		return Element{ true, value };
	}

	static Element createListElement(const std::vector<Element>& elements) {
		return Element{ false, 0, elements };
	}

	int compare(const Element& right) const {
		// both are value
		if (isValue && right.isValue) {
			return intCompare(value, right.value);
		}

		// only one is value
		if (isValue != right.isValue) {
			if (isValue) {
				Element meAsList = createListElement({ *this });
				return meAsList.compare(right);
			}
			Element rightAsList = createListElement({ right });
			return compare(rightAsList);
		}

		// both are list
		const auto commonSize = std::min(elements.size(), right.elements.size());
		for (int i = 0; i < commonSize; ++i) {
			const auto& leftElement = elements[i];
			const auto& rightElement = right.elements[i];
			const auto comparisionResult = leftElement.compare(rightElement);
			if (comparisionResult != 0) {
				return comparisionResult;
			}
		}

		return intCompare(elements.size(), right.elements.size());
	}

	bool isValue{ false };
	int value{ 0 };
	std::vector<Element> elements;
	bool isDivider{ false };
};

struct PacketPair {
	Element left;
	Element right;

	bool isInOrder() const {
		return left.compare(right) >= 0;
	}
};

using DataType = std::vector<PacketPair>;

std::vector<std::string> splitByDelimiter(std::string line, char delimiter) {
	int pos;
	std::vector<std::string> result;
	while ((pos = line.find(delimiter)) != std::string::npos && pos != line.size() - 1) {
		result.push_back(line.substr(0, pos));
		line = line.substr(pos + 1);
	}
	if (!line.empty()) {
		result.push_back(line);
	}
	return result;
}

Element parseElement(const std::string& line) {
	if (line.front() != '[') {
		int value = -1;
		std::istringstream stream(line);
		stream >> value;
		return Element::createValueElement(value);
	}

	std::string content = line.substr(1, line.size() - 2);
	if (content.empty()) {
		return Element::createListElement({});
	}

	std::vector<std::string> splitted = splitByDelimiter(content, ',');
	std::vector<std::string> elementsToParse;
	std::string elementToParse;
	int braceCount = 0;
	for (const auto& fragment : splitted) {
		int startMarkers = std::count(fragment.begin(), fragment.end(), '[');
		int endMarkers = std::count(fragment.begin(), fragment.end(), ']');
		braceCount += startMarkers - endMarkers;
		elementToParse += fragment;
		
		if (braceCount == 0) {
			elementsToParse.push_back(elementToParse);
			elementToParse.clear();
		} else {
			elementToParse += ',';
		}
	}

	std::vector<Element> elements;
	for (const auto& strToParse : elementsToParse) {
		elements.push_back(parseElement(strToParse));
	}
	return Element::createListElement(elements);
}

DataType read() {
	common::FileReader reader("input.txt");
	DataType data;
	for (std::string line; reader.nextLine(line);) {
		if (line.empty()) {
			continue;
		}

		PacketPair pair;
		pair.left = parseElement(line);
		pair.right = parseElement(reader.nextLine());
		data.push_back(std::move(pair));
	}
	return data;
}

int partOne(const DataType& data) {
	int indicesSum = 0;
	for (int i = 0; i < data.size(); ++i) {
		const auto& pair = data[i];
		if (pair.isInOrder()) {
			indicesSum += i + 1;
		}
	}
	return indicesSum;
}

int partTwo(const DataType& data) {
	std::vector<Element> elements;
	elements.reserve(data.size() * 2 + 2);
	for (const auto& pair : data) {
		elements.push_back(pair.left);
		elements.push_back(pair.right);
	}

	auto divider1 = parseElement("[[2]]");
	divider1.isDivider = true;
	elements.push_back(divider1);

	auto divider2 = parseElement("[[6]]");
	divider2.isDivider = true;
	elements.push_back(divider2);

	std::sort(elements.begin(), elements.end(), [](auto&& left, auto&& right) {
		return left.compare(right) > 0;
	});

	auto divider1Iterator = std::find_if(elements.begin(), elements.end(), [](auto&& element) {
		return element.isDivider;
	});

	auto divider2Iterator = std::find_if(std::next(divider1Iterator), elements.end(), [](auto&& element) {
		return element.isDivider;
	});

	auto divider1Pos = std::distance(elements.begin(), divider1Iterator) + 1;
	auto divider2Pos = std::distance(elements.begin(), divider2Iterator) + 1;
	return divider1Pos * divider2Pos;
}

int main(int argc, char** argv) {
	const auto data = read();
	common::measureAndPrint("Part 1", partOne, data);
	common::measureAndPrint("Part 2", partTwo, data);
	return 0;
}