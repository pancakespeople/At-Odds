#pragma once

#include <map>
#include <string>

class TradeGoods {
public:
	TradeGoods() {}

	void addItem(const std::string& item, float num);

	std::string getContentString() const;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_items;
	}

	std::map<std::string, float> m_items;
};