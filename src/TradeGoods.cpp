#include "gamepch.h"
#include "TradeGoods.h"

void TradeGoods::addItem(const std::string& item, float num) {
	m_items[item] += num;
}

std::string TradeGoods::getContentString() const {
	std::string str;
	for (auto& pair : m_items) {
		str += pair.first + " " + std::to_string(pair.second) + "\n";
	}
	return str;
}