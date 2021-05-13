#include "gamepch.h"
#include "Identifiable.h"

unsigned int Identifiable::numObjects = 0;

Identifiable::Identifiable() {
	numObjects++;
	m_id = numObjects;
}