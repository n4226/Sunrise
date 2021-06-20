#pragma once

#include "srpch.h"

namespace sunrise {

	// reduce = order independant 
	// acumulate = order dependant

	// credit: https://stackoverflow.com/questions/13172158/c-split-string-by-line

	std::vector<std::string> SUNRISE_API split_string
		(const std::string& str, const std::string& delimiter);

	std::vector<std::string> SUNRISE_API split_string
		(std::string&& str, std::string&& delimiter);

}