#pragma once
#include <boost/serialization/access.hpp>

struct AsteroidBelt {
	float seed = 0.0f;
	float radius = 0.0f;

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & seed;
		archive & radius;
	}
};