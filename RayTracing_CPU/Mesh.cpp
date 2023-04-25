#include "Mesh.h"

Triangle::Triangle() {}

Triangle::Triangle(vec4 p1, vec4 p2, vec4 p3) : a(p1), b(p2), c(p3) {
	col = { 1.0f, 1.0f, 1.0f, 1.0f };
	normal = (b - a).cross(c - a);
	normal.w = 1.0f;
	normal.normalize();
}

Triangle::Triangle(vec4 p1, vec4 p2, vec4 p3, vec4 col) : a(p1), b(p2), c(p3), col(col) {
	normal = (b - a).cross(c - a);
	normal.w = 1.0f;
	normal.normalize();
}

Triangle::Triangle(vec4 p1, vec4 p2, vec4 p3, texel t1, texel t2, texel t3) : a(p1), b(p2), c(p3), ta(t1), tb(t2), tc(t3) {
	col = { 1.0f, 1.0f, 1.0f, 1.0f };
	normal = (b - a).cross(c - a);
	normal.w = 1.0f;
	normal.normalize();
}

bool Triangle::hit(const Ray& ray, Packet& result) const {
	vec4 E1 = b - a;
	vec4 E2 = c - a;
	vec4 N = E1.cross(E2);
	float det = -ray.d.dot(N);
	if (-1e-6f <= det && det <= 1e-6f) return false;
	float invdet = 1.0 / det;
	vec4 AO = ray.o - a;
	vec4 DAO;
	DAO = AO.cross(ray.d);
	float u = E2.dot(DAO) * invdet;
	float v = -E1.dot(DAO) * invdet;
	float t = AO.dot(N) * invdet;
	if (t < ray.tMax && t > 0.0f && u >= 0.0f && v >= 0.0f && (u + v) <= 1.0f) {
		result.u = u;
		result.v = v;
		result.normal = normal;
		result.last_hit = a + E1 * result.u + E2 * result.v;
		ray.tMax = t;
		return true;
	}
	return false;
}

Bound Triangle::getBound() const {
	vec4 min;
	vec4 max;
	for (int i = 0; i < 3; i++) {
		max.v[i] = std::max(a.v[i], std::max(b.v[i], c.v[i]));
		min.v[i] = std::min(a.v[i], std::min(b.v[i], c.v[i]));
	}
	return Bound(min, max);
}

inline texel getTexcord(const Triangle& tri, const texel& uv) {
	texel texcord = tri.ta + (tri.tb - tri.ta) * uv.x + (tri.tc - tri.ta) * uv.y;
	while (texcord.x > 1.0f) texcord.x -= 1.0f;
	while (texcord.x < 0.0f) texcord.x += 1.0f;
	while (texcord.y > 1.0f) texcord.y -= 1.0f;
	while (texcord.y < 0.0f) texcord.y += 1.0f;
	return texcord;
}

bool Object::hit(const Ray& ray, Packet& result) const {
	const Triangle* closest_triangle = nullptr;
	vec4 invertRayDir = { 1 / ray.d.v(0), 1 / ray.d.v(1), 1 / ray.d.v(2), 1.0f };
	uint32_t iter = 0, stack_size = 0;
	uint32_t stack[64];

	auto& bvh = tree.bvh;

	float tMax = ray.tMax;
	bool hitted = false;
	vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

	// Delete this
	result.shape_check--;

	while (true) {
		// Delete this
		result.aabb_check++;

		if (bvh[iter].bound.hit(ray.o, invertRayDir, ray.tMax)) {
			if (bvh[iter].next_index == 0) {
				auto& tri = mesh[bvh[iter].obj_index];

				result.shape_check++;
				if (tri.hit(ray, result)) {

					if (material == nullptr) closest_triangle = &tri;
					else if (material->texture_map.size() == 0) closest_triangle = &tri;
					else {
						color = material->getTexture(getTexcord(tri, { result.u, result.v }));
						if (color.w > 0.70f) {
							closest_triangle = &tri;
						}
						else {
							ray.tMax = tMax;
						}
					}
				}
				if (stack_size < 1) break;
				iter = stack[--stack_size];
			}
			else {
				if (invertRayDir.v(0) > 0) {
					stack[stack_size++] = bvh[iter++].next_index;
				}
				else {
					stack[stack_size++] = iter + 1;
					iter = bvh[iter].next_index;
				}
			}
		}
		else {
			if (stack_size < 1) break;
			iter = stack[--stack_size];
		}
	}

	if (closest_triangle != nullptr) {
		if (material == nullptr) result.color = closest_triangle->col;
		else {
			texel texCoord = getTexcord(*closest_triangle, { result.u, result.v });
			result.u = texCoord.x;
			result.v = texCoord.y;
			result.material = material;
		}
		result.normal = closest_triangle->normal;
		return true;	
	}
	return false;
}

Bound Object::getBound() const {
	return bound;
}

void Object::buildTree() {
	std::deque<std::unique_ptr<BVHNode>> nodes;
	for (long i = 0; i < mesh.size(); i++) {
		nodes.emplace_back(std::make_unique<BVHNode>(mesh[i].getBound(), i));
	}
	auto root = constructBVH(std::move(nodes), BuildStrat::TOPDOWN);
	size_t i = 0;
	constructLinearBVH(root, tree, i);
}
