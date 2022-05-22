#pragma once

struct Resource {
	Resource() {}
	Resource(const std::string& type);

	std::string type;
	sf::Vector2i pos;
	float abundance = 0.0f; // Value between 0 and 1
	bool hidden = true;

	std::string getName();
	static std::string getResourceString(const std::unordered_map<std::string, float>& resources);
	static std::unordered_map<std::string, float> nodeToMap(const toml::node_view<const toml::node>& node);

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & type;
		archive & abundance;
		archive & hidden;
		archive & pos;
	}
};