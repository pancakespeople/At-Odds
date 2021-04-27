#include "gamepch.h"
#include "Pathfinder.h"
#include "Star.h"

#include <queue>

std::list<Star*> Pathfinder::findPath(Star* begin, Star* end) {
	//sf::Clock benchmarker;
	
	std::vector<Node*> exploredStars;
	std::queue<Node*> toBeExploredStars;
	std::list<Star*> finalPath;

	if (begin == end) {
		finalPath.push_back(begin);
		return finalPath;
	}

	Node* root = new Node;
	root->star = begin;
	root->prevNode = nullptr;
	root->dist = 0;

	toBeExploredStars.push(root);

	int dist = 1;

	// Explore nodes with nodes closer to root being first
	while (toBeExploredStars.size() > 0) {
		Star* currentStar = toBeExploredStars.front()->star;

		if (currentStar == end) {
			exploredStars.push_back(toBeExploredStars.front());
			toBeExploredStars.pop();
			break;
		}
		
		std::vector<Star*> connectedStars = currentStar->getConnectedStars();
		
		for (Star* s : connectedStars) {
			
			// If star is not explored already
			if (std::find_if(exploredStars.begin(), exploredStars.end(), [s](Node* n) {return n->star == s; }) == exploredStars.end()) {
				Node* n = new Node;
				n->star = s;
				n->prevNode = toBeExploredStars.front();
				n->dist = dist;
				toBeExploredStars.push(n);
			}
		}

		exploredStars.push_back(toBeExploredStars.front());
		toBeExploredStars.pop();
		dist++;

		// Valid path was not found
		if (toBeExploredStars.size() == 0) {
			DEBUG_PRINT("Unable to find path");
			finalPath.push_back(begin);
			return finalPath;
		}
	}

	// Crawl back to the beginning with our handy pointers
	Node* n = exploredStars.back();
	while (n != nullptr) {
		finalPath.push_back(n->star);
		n = n->prevNode;
	}
	finalPath.reverse();

	while (toBeExploredStars.size() > 0) {
		delete toBeExploredStars.front();
		toBeExploredStars.pop();
	}
	for (Node* n : exploredStars) {
		delete n;
	}

	//DEBUG_PRINT("Pathfinding complete");
	//DEBUG_PRINT("Time elapsed: " << benchmarker.getElapsedTime().asSeconds() << " seconds");

	return finalPath;
}