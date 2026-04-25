#include "Scene.h"

void Scene::buildTree() {
	std::deque<std::unique_ptr<BVHNode>> nodes;
	
	for (size_t i = 0; i < objects.size(); i++) {
		nodes.emplace_back(std::make_unique<BVHNode>(objects[i]->getBound(), i));
		nodes.back()->cost = objects[i]->getCost();
	}

	auto bvh = constructBVH(std::move(nodes), BuildStrat::TOPDOWN);
	
	size_t i = 0;
	tree.bvh.clear();
	constructLinearBVH(bvh, tree, i);
}
