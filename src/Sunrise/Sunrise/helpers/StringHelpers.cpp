#include "srpch.h"
#include "StringHelpers.h"

/// <summary>
/// not tht fast
/// </summary>
/// <param name="s"></param>
/// <param name="delimiter"></param>
/// <returns></returns>
std::vector<std::string> sunrise::helpers::split(const std::string& s, const std::string& delimiter) {
https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c

	std::vector<std::string> result{};

	size_t startPos = 0;
	size_t pos = 0;
	std::string token;
	while ((pos = s.find(delimiter, startPos)) != std::string::npos) {
		token = s.substr(startPos, pos);
		result.push_back(token);

		startPos += pos + delimiter.length();
		//s.erase(0, pos + delimiter.length());
	}
	result.push_back(s.substr(startPos));
	return result;
}
