#pragma once

#include <string>
#include <vector>

class Utility
{

public:

	static void ParseString(std::string& string,
		std::vector<std::string>& subStrings, char token);

};