#pragma once

#include "SaveLoader.h"

// This class gives all derived classes an ID
class Identifiable {
public:

	Identifiable();

	unsigned int getID() { return m_id; }

private:
	friend class boost::serialization::access;
	friend class SaveLoader;
	template<class Archive>
	void serialize(Archive& archive, const unsigned int version) {
		archive & m_id;
	}
	
	static unsigned int numObjects;

	unsigned int m_id = 0;
};