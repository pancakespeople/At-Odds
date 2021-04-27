#pragma once
#include <vector>

class Star;

class Pathfinder {
public:
	struct Node {
		Star* star;
		Node* prevNode;
		int dist;
	};

	static std::list<Star*> findPath(Star* begin, Star* end);
};

