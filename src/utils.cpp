#include <sstream>
#include <iomanip>
#include <fstream>
#include <iterator>
#include <iostream>

#include "utils.h"

using namespace std;

string sizeToString(size_t size)
{
    stringstream ss;

    if(size < 1024)
        ss <<  size << " B";
    else if(size < 1024 * 1024)
        ss << fixed << setprecision(1) << (size / 1024.0) << " KiB";
    else if(size < 1024 * 1024 * 1024)
        ss << fixed << setprecision(1) << (size / 1024.0 / 1024.0) << " MiB";
    else
        ss << fixed << setprecision(1) << (size / 1024.0 / 1024.0 / 1024.0) << " GiB";

    return ss.str();
}

int roundToInt(float f)
{
	return (int)(f + 0.5f);
}

bool readFile(const std::string& fileName, string& buffer)
{
	ifstream sourceFile(fileName, ios::binary | ios::in);
	if(!sourceFile)
	{
		cerr << "Error opening file " << fileName << endl;
		return false;
	}

	buffer = string((istreambuf_iterator<char>(sourceFile)), istreambuf_iterator<char>());

	sourceFile.close();

	return true;
}