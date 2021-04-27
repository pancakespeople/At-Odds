#pragma once

#include <iostream>

#ifndef NDEBUG
#define DEBUG_PRINT(X) (std::cout << "DEBUG(" << __func__ << "): " << X << "\n")
#else
#define DEBUG_PRINT(X)
#endif

#ifndef NDEBUG
#define AI_DEBUG_PRINT(X) (DEBUG_PRINT("Faction " << faction->getID() << ": " << X))
#else
#define AI_DEBUG_PRINT(X)
#endif