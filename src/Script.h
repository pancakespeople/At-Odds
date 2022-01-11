#pragma once
#define SOL_ALL_SAFETIES_ON 1
#include "ext/sol.hpp"

class Script {
public:
	
	// Filepath is relative to data/scripts
	static bool RunScript(const std::string& filePath);

	template <typename ReturnType, typename... Args>
	static ReturnType CallFunction(const std::string& name, Args&&... args) {
		sol::protected_function_result result = lua[name](args...);
		if (!result.valid()) {
			sol::error err = result;
			DEBUG_PRINT("Script error: " << err.what());
		}
		
		if constexpr (!std::is_void<ReturnType>::value) {
			return result;
		}
	}

private:
	inline static bool initialized = false;
	inline static sol::state lua;
};