#pragma once
#include <string>
#include <iomanip>
#include <sstream>

#include "GameState.h"

namespace Util {
	// Returns a string of a float multiplier as a percent. Ex: 1.75 = 75%, 0.5 = -50%
	static std::string percentify(float num, int precision = 2) {
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

		if (hours < 10) {
			str << "0";
		}
		str << hours << ":";

		if (minutes < 10) {
			str << "0";
		}
		str << minutes << ":";

		if (static_cast<int>(seconds) < 10) {
			str << "0";
		}
		str << static_cast<int>(seconds);

		return str.str();
	}

	static std::string ticksToTime(int ticks) {
		return secondsToTime(ticks / GameState::DEFAULT_UPDATE_TARGET);
	}

	static std::string toRomanNumeral(int num) {
		std::vector<std::string> romanNums = { "M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I" };
		std::vector<int> nums = { 1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1 };

		std::string result;

		for (int i = 0; i < 13; i++) {
			while (num - nums[i] >= 0) {
				result += romanNums[i];
				num -= nums[i];
			}
		}

		return result;
	}

	static std::string cutOffDecimal(float num, int numPoints) {
		std::stringstream ss;
		ss << std::fixed;
		ss << std::setprecision(numPoints);
		ss << num;
		return ss.str();
	}
}