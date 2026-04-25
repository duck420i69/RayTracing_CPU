#include "threading.h"
#include "loader/loader.h"
#include "input.h"

#include "GLFW/GLFW3.h"
#include <cstdlib>

#if 0
int main() {
	Scene scene;
	scene.environment = loadEnvHDR("skybox/kloppenheim_05_puresky_2k.hdr");
	//scene.environment = std::make_shared<DefaultEnvironment>(light_direction, 0.99f, 10.0f);
	//scene.environment = std::make_shared<DefaultEnvironment>(light_direction, -1.0f, 10.0f);
	//scene.environment = loadEnvHDR("skybox/syferfontein_6d_clear_puresky_4k.hdr");
	//scene.environment = loadEnvHDR("skybox/san_giuseppe_bridge_4k.hdr");

	//vec4 direction = { random0to1(), random0to1(), random0to1(), 1.0f };
	//vec4 direction = { 0.999f, 0.01f, 0.0f, 1.0f };
	vec4 direction = { 0.0f, 1.0f, 0.0f, 1.0f };
	direction.normalize();

	Camera cam = Camera(vec4(0,0,0,0) - direction, direction, { 1, 1 }, 45.0f);
	auto obj = std::make_shared<Object>();
	obj->mesh.push_back(Triangle(vec4(1, 0, 0, 1), vec4(-1, 0, 1, 1), vec4(-1, 0, -1, 1)));
	obj->buildTree();
	scene.objects.push_back(obj);
	scene.tree.bvh.push_back(BVHLinearNode(obj->getBound(), 0));
	
	MaterialBuilder matBuilder;
	matBuilder.setSpecularColor(vec4(0.0f, 0.0f, 0.0f, 1.0f));

	RayStackTrace debugger;
	Packet packet;
	packet.normal = obj->mesh[0].normal;
	packet.material = matBuilder.build();
	packet.debugger = (StackTrace*) &debugger;

	Ray next_ray = Ray(cam.pos, cam.dir);

	SampleResult sample;

	vec4 color;

	for (long j = 0; j < 100; j++) {
		vec4 s1 = { 0.0f, 0.0f, 0.0f, 0.0f };
		vec4 s2 = { 0.0f, 0.0f, 0.0f, 0.0f };

		for (long i = 0; i < 100000; i++) {
			sample = RIS(scene, Ray(cam.pos, cam.dir), packet);
			packet.material->interact(next_ray, sample, packet);
			color = packet.color;
			scene.environment->interact(Ray(cam.pos, sample.sample), packet);
			s1 = s1 + color * packet.color * sample.inv_pdf;
		}
		for (long i = 0; i < 100000; i++) {
			sample = random_cos_weight(packet.normal);
			packet.material->interact(next_ray, sample, packet);
			color = packet.color;
			scene.environment->interact(Ray(cam.pos, sample.sample), packet);
			s2 = s2 + color * packet.color * sample.inv_pdf;
		}
		//std::cout << to_string(s1) << " ?= " << to_string(s2);
		std::cout << s1.x() / 100000 << " - " << s2.x() / 100000 << " = " << s1.x() / 100000 - s2.x() / 100000 << std::endl;
	}
}
#endif