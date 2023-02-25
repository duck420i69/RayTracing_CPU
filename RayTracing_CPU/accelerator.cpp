#include "accelerator.h"
#include <algorithm>

std::unique_ptr<BVHNode> constructBVH(std::deque<std::unique_ptr<BVHNode>> component) {
	while (component.size() > 1) {
		int min_index = 0;
		float min_cost = INFINITY;

		for (int i = 1; i < component.size(); i++) {
			float cost = (component[0]->bound.surfaceArea() * component[0]->cost + (component[i]->bound.surfaceArea() * component[i]->cost))
				/ Union(component[0]->bound, component[i]->bound).surfaceArea();
			if (cost < min_cost) {
				min_index = i;
				min_cost = cost;
			}
		}
		auto newNode = std::make_unique<BVHNode>(Union(component[0]->bound, component[min_index]->bound), 0);
		newNode->cost = min_cost + 0.125f;
		newNode->child.emplace_back(std::move(component[0]));
		newNode->child.emplace_back(std::move(component[min_index]));
		if (newNode->child[0]->centroid[0] > newNode->child[1]->centroid[0])
			std::swap(newNode->child[0], newNode->child[1]);
		component.emplace_back(std::move(newNode));
		component.erase(component.begin() + min_index);
		component.erase(component.begin());
	}
	return std::move(component.front());
}

void constructLinearBVH(const std::unique_ptr<BVHNode>& root, BVHLinear& arr, size_t& iter) {
	int current = iter;
	arr.bvh.push_back(BVHLinearNode(root->bound, root->obj_index));
	arr.bvh[current].next_index = 0;
	if (root->child.size() > 0) {
		constructLinearBVH(std::move(root->child[0]), arr, ++iter);
		arr.bvh[current].next_index = ++iter;
		constructLinearBVH(std::move(root->child[1]), arr, iter);
	}
}
