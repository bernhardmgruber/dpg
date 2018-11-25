#pragma once

#include <iomanip>
#include <sstream>
#include <string>

std::string sizeToString(size_t size);

int roundToInt(float f);

bool readFile(const std::string& fileName, std::string& buffer);

template<typename T>
std::string toHexString(T val) {
	stringstream ss;
	ss << hex << val;
	return ss.str();
}