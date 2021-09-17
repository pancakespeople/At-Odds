#pragma once
#include <string>
#include <iomanip>
#include <sstream>

namespace Util {
	// Returns a string of a float multiplier as a percent. Ex: 1.75 = 75%, 0.5 = -50%
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

	// Converts seconds to a timestamp
	static std::string secondsToTime(float seconds) {
		int minutes = seconds / 60.0f;
		int hours = minutes / 60.0f;
		minutes = minutes % 60;
		seconds = static_cast<int>(seconds) % 60;

		std::stringstream str;

		str << hours << ":" << minutes << ":" << static_cast<int>(seconds);
		return str.str();
	}
}