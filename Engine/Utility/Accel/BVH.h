#pragma once

#include "AABB.h"
#include <set>

class Graphic;

struct BVH
{
	struct Node
	{
		Node(int _is_leaf, int _data, AABB _bounds);

		int is_leaf: 1;
		int child: 31;

        AABB bounds;
	};

	void insert(Graphic *g);
	void update();


	int depth();

	private:
		void refit(int i);
		void rotate_node(int i);

		std::vector<Node> nodes;
		std::set<int> free_nodes;
		std::vector<Graphic*> graphics;
};
