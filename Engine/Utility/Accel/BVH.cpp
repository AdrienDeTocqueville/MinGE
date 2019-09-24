#include "Utility/Accel/BVH.h"
#include "Components/Graphic.h"

BVH::Node::Node(int _is_leaf, int _child, AABB _bounds):
	is_leaf(_is_leaf), child(_child), bounds(_bounds)
{ }

void BVH::insert(Graphic *g)
{
	if (!g->getMesh())
		return;

	AABB bounds = g->getAABB();
	graphics.push_back(g);

	if (nodes.empty())
	{
		nodes.emplace_back(1, 0, bounds);
		return;
	}

	int i = 0;
	while (!nodes[i].is_leaf)
	{
		float costs[3];
		const int left = nodes[i].child;
		const int right = nodes[i].child + 1;

		// 1st case: merge with left node
		{
			AABB box = nodes[left].bounds; box.extend(bounds);
			costs[0] = nodes[right].bounds.volume() + box.volume();
		}

		// 2nd case: merge with right node
		{
			AABB box = nodes[right].bounds; box.extend(bounds);
			costs[1] = nodes[left].bounds.volume() + box.volume();
		}

		// 3rd case: create new node
		{
			costs[2] = bounds.volume() + nodes[i].bounds.volume();
		}

		// Find best case
		const int parent = i;
		if (costs[0] < costs[2])
		{
			if (costs[0] < costs[1]) // 1st
				i = left;
			else //2nd
				i = right;
		}
		else if (costs[1] < costs[2]) //2nd
			i = right;
		else // 3rd
			break;
		nodes[parent].bounds.extend(bounds);
	}

	int child_index = nodes.size();
	nodes.emplace_back(nodes[i]);
	nodes.emplace_back(1, graphics.size() - 1, bounds);
	// Update parent
	nodes[i].is_leaf = 0;
	nodes[i].child = child_index;
	nodes[i].bounds.extend(bounds);
}

static int _depth(std::vector<BVH::Node> &nodes, int i)
{
	if (nodes[i].is_leaf)
		return 0;
	return 1 + max(_depth(nodes, nodes[i].child), _depth(nodes, nodes[i].child + 1));
}

int BVH::depth()
{
	return _depth(nodes, 0);
}

// "Fast, Effective BVH Updates for Animated Scenes"
void BVH::rotate_node(int i)
{
	const int left = nodes[i].child;
	const int right = left + 1;

	const int left_left = nodes[left].child;
	const int left_right = left_left + 1;
	const int right_left = nodes[right].child;
	const int right_right = right_left + 1;

	const float left_vol = nodes[left].bounds.volume();
	const float right_vol = nodes[right].bounds.volume();

	AABB box, box2;
	AABB best_box, best_box2;
	int swap_a, swap_b, swap_refit;
	int best = 0;
	float cost, best_cost = left_vol + right_vol;

	/// Find best rotation
	if (!nodes[left].is_leaf)
	{
		const float ll_vol = nodes[left_left].bounds.volume();
		const float lr_vol = nodes[left_right].bounds.volume();

		// (1)    N                     N      //
		//       / \                   / \     //
		//      L   R     ----->      L   LL   //
		//     / \                   / \       //
		//   LL   LR                R   LR     //
		box = nodes[right].bounds; box.extend(nodes[left_right].bounds);
		cost = box.volume() + ll_vol;
		if (cost < best_cost)
		{
			best = 1;
			best_cost = cost;
			best_box = box;
			// Swap LL <-> R
			swap_refit = left;
			swap_a = left_left;
			swap_b = right;
		}

		// (2)    N                     N      //
		//       / \                   / \     //
		//      L   R     ----->      L   LR   //
		//     / \                   / \       //
		//   LL   LR               LL   R      //
		box = nodes[right].bounds; box.extend(nodes[left_left].bounds);
		cost = box.volume() + lr_vol;
		if (cost < best_cost)
		{
			best = 2;
			best_cost = cost;
			best_box = box;
			// Swap LR <-> R
			swap_refit = left;
			swap_a = left_right;
			swap_b = right;
		}
	}

	if (!nodes[right].is_leaf)
	{
		const float rl_vol = nodes[right_left].bounds.volume();
		const float rr_vol = nodes[right_right].bounds.volume();

		// (3)    N                     N        //
		//       / \                   / \       //
		//      L   R     ----->     RL   R      //
		//         / \                   / \     //
		//       RL   RR                L   RR   //
		box = nodes[left].bounds; box.extend(nodes[right_right].bounds);
		cost = rl_vol + box.volume();
		if (cost < best_cost)
		{
			best = 3;
			best_cost = cost;
			best_box = box;
			// Swap RL <-> L
			swap_refit = right;
			swap_a = right_left;
			swap_b = left;
		}

		// (4)    N                     N       //
		//       / \                   / \      //
		//      L   R     ----->     RR   R     //
		//         / \                   / \    //
		//       RL   RR               RL   L   //
		box = nodes[left].bounds; box.extend(nodes[right_left].bounds);
		cost = rr_vol + box.volume();
		if (cost < best_cost)
		{
			best = 4;
			best_cost = cost;
			best_box = box;
			// Swap RR <-> L
			swap_refit = right;
			swap_a = right_right;
			swap_b = left;
		}

		if (!nodes[left].is_leaf)
		{
			// (5)       N                      N        //
			//         /   \                  /   \      //
			//        L     R     ----->     L     R     //
			//       / \   / \              / \   / \    //
			//      LL LR RL RR            LL RL LR RR   //
			box = nodes[left_left].bounds;
			box.extend(nodes[right_left].bounds);
			box2 = nodes[left_right].bounds;
			box2.extend(nodes[right_right].bounds);
			cost = box.volume() + box2.volume();
			if (cost < best_cost)
			{
				best = 5;
				best_cost = cost;
				best_box = box;
				best_box2 = box2;
				swap_a = left_right;
				swap_b = right_left;
			}

			// (6)       N                      N        //
			//         /   \                  /   \      //
			//        L     R     ----->     L     R     //
			//       / \   / \              / \   / \    //
			//      LL LR RL RR            LL RR RL LR   //
			box = nodes[left_left].bounds;
			box.extend(nodes[right_right].bounds);
			box2 = nodes[right_left].bounds;
			box2.extend(nodes[left_right].bounds);
			cost = box.volume() + box2.volume();
			if (cost < best_cost)
			{
				best = 6;
				best_cost = cost;
				best_box = box;
				best_box2 = box2;
				swap_a = left_right;
				swap_b = right_right;
			}
		}
	}

	/// Apply rotation
	if (best == 0)
		return;

	if (best <= 4) // case 1, 2, 3, 4
	{
		std::swap(nodes[swap_a], nodes[swap_b]);
		nodes[swap_refit].bounds = best_box;
	}
	else // case 5, 6
	{
		std::swap(nodes[swap_a], nodes[swap_b]);
		nodes[left].bounds = best_box;
		nodes[right].bounds = best_box2;
	}
}

#ifdef DRAWAABB
static int prepare(std::vector<BVH::Node> &nodes, int i)
{
	if (nodes[i].is_leaf)
	{
		nodes[i].bounds.prepare();
		return 0;
	}

	int num = max(
		prepare(nodes, nodes[i].child),
		prepare(nodes, nodes[i].child + 1)
	);
	nodes[i].bounds.prepare(num * 0.1f);
	return 1 + num;
}
#endif

void BVH::update()
{
	refit(0);
#ifdef DRAWAABB
	prepare(nodes, 0);
#endif
}

void BVH::refit(int i)
{
	if (nodes[i].is_leaf)
	{
		AABB box = graphics[nodes[i].child]->getAABB();
		nodes[i].bounds = box;
		return;
	}

	const int left = nodes[i].child;
	const int right = left + 1;

	refit(left);
	nodes[i].bounds = nodes[left].bounds;

	refit(right);
	nodes[i].bounds.extend(nodes[right].bounds);

	rotate_node(i);
}
