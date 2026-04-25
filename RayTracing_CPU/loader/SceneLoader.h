#pragma once


#include "../Scene.h"

#include "../Camera.h"
#include "../Environment.h"
#include "../material/MaterialBuilder.h"

#include <fstream>


enum class SceneBlobType : uint8_t {
	OBJECTS,
	MATERIALS,
	LIGHTSOURCES,
	ENVIRONMENT,
	CAMERA
};

struct SceneFileHeader {
	char signature[6];
	uint16_t version;
	uint64_t fileSize;
	uint64_t contentBegin;
};

template<class T, class SubType>
struct SceneFileSingle {
	uint64_t size;
	SubType type;
	const T* data;
};

template<class T, class SubType>
struct SceneFileArray {
	uint32_t size;
	SubType type;
	const T* data;
};


template<class T>
struct SceneFileBody {
	uint64_t size;
	SceneBlobType type;
	T data;
};


struct CameraSaveData {
	vec3 pos;
	vec3 dir;
	float fov;

	static CameraSaveData convert(const Camera& camera) {
		return {
			camera.pos,
			camera.dir,
			camera.fov
		};
	}
};


void saveScene(const std::string& filename, const Scene& scene) {
	std::ofstream file = std::ofstream(filename + ".rtscene", std::ios::out | std::ios::binary | std::ios::app);

	SceneFileHeader header{};
	header.signature[0] = 'R';
	header.signature[1] = 'T';
	header.signature[2] = 'S';
	header.signature[3] = 'C';
	header.signature[4] = 'N';
	header.signature[5] = 'E';

	const uint8_t majorVersion = 1;
	const uint8_t minorVersion = 0;

	header.version = majorVersion << 8 + minorVersion;
	header.contentBegin = sizeof(SceneFileHeader);

	SceneFileBody<CameraSaveData> camera{};
	camera.type = SceneBlobType::CAMERA;
	camera.size = sizeof(CameraSaveData);
	camera.data = CameraSaveData::convert(scene.camera);

	SceneFileBody<SceneFileArray<Material, MaterialType>> materials{};
	materials.type = SceneBlobType::MATERIALS;
	materials.size = sizeof(CameraSaveData);

	SceneFileBody<SceneFileArray<Hittable, HittableType>> objects{};
	objects.type = SceneBlobType::OBJECTS;
	objects.size = sizeof(CameraSaveData);

	SceneFileBody<SceneFileSingle<Environment, EnvironmentType>> environment{};
	environment.type = SceneBlobType::ENVIRONMENT;
	environment.size = sizeof(CameraSaveData);
}

void loadScene(std::string filename) {

}