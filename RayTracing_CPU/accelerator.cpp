#include "accelerator.h"
#include "Sampling.h"
#include <algorithm>

std::unique_ptr<BVHNode> constructBVH_SAH(std::deque<std::unique_ptr<BVHNode>> component) {
	std::sort(component.begin(), component.end(), [](const std::unique_ptr<BVHNode>& b1, const std::unique_ptr<BVHNode>& b2) -> bool {
		return b1->centroid(0) < b2->centroid(0);
		});

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

std::unique_ptr<BVHNode> constructBVH_Topdown(std::deque<std::unique_ptr<BVHNode>>& component, size_t begin, size_t end) {
	if (end - begin <= 0) return nullptr;
	if (end - begin == 1) {
		return std::move(component[begin]);
	}
	if (end - begin == 2) {
		auto newNode = std::make_unique<BVHNode>(Union(component[begin]->bound, component[begin+1]->bound), 0);
		newNode->child.emplace_back(std::move(component[begin]));
		newNode->child.emplace_back(std::move(component[begin + 1]));
		return newNode;
	}
	int dim = (int)(random0to1() * 3.0f) % 3;
	size_t mid_point = (begin + end) / 2;
	std::nth_element(component.begin() + begin, component.begin() + mid_point, component.begin() + end,
		[dim](const std::unique_ptr<BVHNode>& comp1, const std::unique_ptr<BVHNode>& comp2) {
			return comp1->centroid[dim] < comp2->centroid[dim];
		});
	auto newNode = std::make_unique<BVHNode>(Bound(), 0);
	newNode->child.emplace_back(std::move(constructBVH_Topdown(component, begin, mid_point)));
	newNode->child.emplace_back(std::move(constructBVH_Topdown(component, mid_point, end)));
	newNode->bound = Union(newNode->child[0]->bound, newNode->child[1]->bound);
	if (newNode->child[0]->centroid[0] > newNode->child[1]->centroid[0])
		std::swap(newNode->child[0], newNode->child[1]);
	return newNode;
}


std::unique_ptr<BVHNode> constructBVH(std::deque<std::unique_ptr<BVHNode>> component, BuildStrat buildStrat) {
	if (buildStrat == BuildStrat::SAH)
		return constructBVH_SAH(std::move(component));
	if (buildStrat == BuildStrat::TOPDOWN)
		return constructBVH_Topdown(component, 0, component.size());
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
