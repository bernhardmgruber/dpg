#pragma once

#include <string>

std::string sizeToString(size_t size);

int roundToInt(float f);

bool readFile(const std::string& fileName, std::string& buffer);
