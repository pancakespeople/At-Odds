#pragma once
#include <string>

class Tech {
public:
	Tech(const std::string& type);
	std::string getName() const;
	std::string getDescription() const;
	std::string getType() const { return m_type; }

private:
	std::string m_type;
};