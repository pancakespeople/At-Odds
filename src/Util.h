#pragma once
#include <string>
#include <iomanip>
#include <sstream>

namespace Util {
	static std::string percentify(float num, int precision) {
		std::string plus;
		std::stringstream str;

		str << std::fixed << std::setprecision(precision);

		if (num > 1.0f) {
			plus = "+";
		}
		else {
			plus = "";
		}

		str << plus << (num - 1.0) * 100.0f << "%";
		return str.str();
	}
}