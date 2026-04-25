#include "../accelerator/accelerator.h"
#include "../material/MaterialBuilder.h"

#include "loader.h"

#include <chrono>
#include <iostream>
#include <malloc.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../dependency/stb_image.h"


std::vector<std::shared_ptr<Material>> loadMTL(std::filesystem::path filename) {
	std::vector<std::shared_ptr<Material>> data;
	MaterialBuilder builder;
	bool hasMaterial = false;

	std::ifstream file;
	file.open(filename);

	if (file.is_open()) {
		while (!file.eof()) {
			std::string str;
			getline(file, str);

			std::stringstream input(str);
			input >> str;

			if (str == "newmtl") {
				if (hasMaterial)
					data.emplace_back(builder.build());

				hasMaterial = true;
				input >> str;
				builder.setName(str);
			}
			else if (str == "Ka") {
				Color color = { 0.0f, 0.0f, 0.0f, 1.0f };
				for (int i = 0; i < 3; i++) input >> color.v(i);
				builder.setAmbientColor(color);
			}
			else if (str == "Kd") {
				Color color = { 0.0f, 0.0f, 0.0f, 1.0f };
				for (int i = 0; i < 3; i++) input >> color.v(i);
				builder.setDiffuseColor(color);
			}
			else if (str == "Ks") {
				Color color = { 0.0f, 0.0f, 0.0f, 1.0f };
				for (int i = 0; i < 3; i++) input >> color.v(i);
				builder.setSpecularColor(color);
			}
			else if (str == "Ni") {
				float refraction;
				input >> refraction;
				builder.setRefraction(refraction);
			}
			else if (str == "Ns") {
				float specular_exp;
				input >> specular_exp;
				builder.setShininess(specular_exp);
			}
			else if (str == "map_Kd") {
				input >> str;
				int channel = 4;
				int width, height;
				std::vector<Color> texture_map;

				float* image = nullptr;
				if (str[1] == ':') {
					std::cout << str << "\n\n";
					image = stbi_loadf(str.c_str(), &width, &height, &channel, 4);
				}
				else {
					std::cout << str << "\n\n";
					image = stbi_loadf((filename.parent_path() / str).string().c_str(), &width, &height, &channel, 4);
				}

				if (image == nullptr) {
					std::cout << stbi_failure_reason();
				}

				texture_map.resize(width * height);
				memcpy(texture_map.data(), image, sizeof(Color) * width * height);
				builder.setTexture(width, height, std::move(texture_map));

				stbi_image_free(image);
			}
		}
		if (hasMaterial)
			data.emplace_back(builder.build());
	}
	else {
		std::cout << "Unable to load material. file: " << filename;
		throw std::runtime_error("Unable to load material. file: " + filename.string());
	}
	return data;
}

std::shared_ptr<EnvironmentHDR> loadEnvHDR(std::string filename) {
	std::filesystem::path path(filename);
	path = std::filesystem::current_path() / path;
	int width, height, comp;
	auto data = stbi_loadf(path.string().c_str(), &width, &height, &comp, 4);
	if (data == nullptr) {
		std::cout << "Cant load HDR file: " << filename << std::endl;
		throw std::runtime_error("Cant load HDR file: " + filename);
	}
	std::vector<Color> env_map;
	float surface_area = 0.0f;
	env_map.resize(width * height);
	for (int i = 0; i < width * height; i++) {
		if (i % width == 0)
			surface_area = (cos(i / width / (float) height * pi) - cos((i / width + 1) / (float) height * pi)) / width;
		env_map[i].x() = data[i * 4];
		env_map[i].y() = data[i * 4 + 1];
		env_map[i].z() = data[i * 4 + 2];
		env_map[i].w = surface_area * env_map[i].v.norm();  // used as probability to choose the pixel
	}
	free(data);
	return std::make_shared<EnvironmentHDR>(std::move(env_map), width, height);
}


std::string getToken(const std::string& str, size_t& offset, char lim = ' ') {
	auto newOffset = str.find(lim, offset);
	std::string token = str.substr(offset, newOffset - offset);
	offset = ++newOffset;
	return token;
}

Scene loadobj(std::string filename) {
	std::shared_ptr<Object> newObject = std::make_shared<Object>();
	std::shared_ptr<Material> currentMat;
	std::vector<vec3> vertices;
	std::vector<texel> vertex;

	Scene scene;

	std::shared_ptr<Matte> default_material = std::make_shared<Matte>();
	default_material->diffuse_color = { 0.256f, 0.256f, 0.256f, 1.0f };
	currentMat = default_material;

	stbi_ldr_to_hdr_gamma(1.0f);

	auto start = std::chrono::system_clock::now();
	auto end = start;

	long long build_tree_time = 0;
	long long read_file_time = 0;

	std::ifstream file;
	std::filesystem::path path(filename);
	path = std::filesystem::current_path() / path;
	file.open(path);

	if (file.is_open()) {
		while (!file.eof()) {
			start = std::chrono::system_clock::now();

			std::string str;
			size_t offset = 0;
			getline(file, str);

			std::string token = getToken(str, offset);

			if (token == "mtllib") {
				str = getToken(str, offset);
				scene.materials = loadMTL(path.parent_path() / str);
			}
			else if (token == "usemtl") {
				if (newObject->mesh.size() == 0) {
					newObject = std::make_shared<Object>();
					continue;
				}

				newObject->material = currentMat;
				if (newObject->material == nullptr)
					newObject->material = default_material;

				str = getToken(str, offset);
				for (auto& mat : scene.materials) {
					if (mat->name == str) {
						currentMat = mat;
						break;
					}
				}

				end = std::chrono::system_clock::now();
				auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
				read_file_time += duration.count();

				start = std::chrono::system_clock::now();
				newObject->buildTree();
				end = std::chrono::system_clock::now();

				duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
				build_tree_time += duration.count();

				start = end;

				scene.objects.emplace_back(newObject);
				newObject = std::make_shared<Object>();
			}
			else if (token == "g") {

			}
			else if (token == "v") {
				vec3 v;
				for (int i = 0; i < 3; i++) {
					token = getToken(str, offset);
					v(i) = std::stof(token);
				}
				vertices.emplace_back(v);
			}
			else if (token == "vt") {
				texel temp;
				token = getToken(str, offset);
				temp.x = std::stof(token);
				token = getToken(str, offset);
				temp.y = std::stof(token);
				vertex.emplace_back(temp);
			}
			else if (token == "f") {
				bool hasMaterial = false;

				std::vector<size_t> v, vt;

				while (offset != 0) {
					auto token = getToken(str, offset);
					size_t offset2 = 0;
					v.push_back(std::stoi(getToken(token, offset2, '/')));

					auto tokeni = getToken(token, offset2, '/');
					if (tokeni.empty()) continue;
					vt.push_back(std::stoi(tokeni));
					hasMaterial = true;

					tokeni = getToken(token, offset2, ' ');
					if (tokeni.empty()) continue;
					// std::stoi(tokeni);
				}

				if (!hasMaterial) {
					texel t = { 0.0f, 0.0f };
					newObject->mesh.emplace_back(Triangle(vertices[v[0] - 1], vertices[v[1] - 1], vertices[v[2] - 1], t, t, t));
				}
				else {
					for (size_t i = 0; i + 2 < v.size(); i++) {
						if (vt.size() == v.size()) {
							newObject->mesh.emplace_back(
								Triangle(vertices[v[i] - 1], vertices[v[i + 1] - 1], vertices[v[i + 2] - 1],
										 vertex[vt[i] - 1], vertex[vt[i + 1] - 1], vertex[vt[i + 2] - 1])
							);
							newObject->bound = Union(newObject->bound, newObject->mesh.back().getBound());
						}
						else {
							newObject->mesh.emplace_back(
								Triangle(vertices[v[i] - 1], vertices[v[i + 1] - 1], vertices[v[i + 2] - 1])
							);
							newObject->bound = Union(newObject->bound, newObject->mesh.back().getBound());
						}
					}
				}
			}

			end = std::chrono::system_clock::now();

			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
			read_file_time += duration.count();
		}

		newObject->buildTree();

		if (newObject->material == nullptr)
			newObject->material = default_material;

		scene.objects.emplace_back(newObject);

		scene.materials.emplace_back(default_material);

		file.close();

		end = std::chrono::system_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		read_file_time += duration.count();

		MaterialBuilder builder;
		auto sphere = std::make_shared<Sphere>();
		sphere->center = { 0.0f, 2.0f, 10.0f };
		sphere->r = 2.0f;
		sphere->material = builder.setDiffuseColor({ 1.0f, 0.256f, 0.256f, 1.0f })
			.setSpecularColor({ 1.0f, 1.0f, 1.0f, 1.0f })
			.setShininess(1000.0f)
			.setRefraction(1.5f)
			.build();
		scene.objects.emplace_back(sphere);

		sphere = std::make_shared<Sphere>();
		sphere->center = { 0.0f, 2.0f, 15.0f };
		sphere->r = 2.0f;
		sphere->material = builder.setDiffuseColor({ 1.0f, 0.256f, 0.256f, 1.0f })
			.setSpecularColor({ 0.956f, 0.956f, 0.956f, 1.0f })
			.setShininess(400.0f)
			.setRefraction(1.5f)
			.build();
		scene.objects.emplace_back(sphere);

		sphere = std::make_shared<Sphere>();
		sphere->center = { 0.0f, 2.0f, 20.0f };
		sphere->r = 2.0f;
		sphere->material = builder.setDiffuseColor({ 1.0f, 0.256f, 0.256f, 1.0f })
			.setSpecularColor({ 0.956f, 0.956f, 0.956f, 1.0f })
			.setShininess(200.0f)
			.setRefraction(1.5f)
			.build();
		scene.objects.emplace_back(sphere);

		sphere = std::make_shared<Sphere>();
		sphere->center = { 10.0f, 2.0f, 25.0f };
		sphere->r = 2.0f;
		sphere->material = builder.setDiffuseColor({ 1.0f, 0.256f, 0.256f, 1.0f })
			.setSpecularColor({ 0.956f, 0.956f, 0.956f, 1.0f })
			.setShininess(0.0f)
			.setRefraction(1.5f)
			.build();
		scene.objects.emplace_back(sphere);

		start = std::chrono::system_clock::now();
		scene.buildTree();
		end = std::chrono::system_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		build_tree_time += duration.count();

		std::cout << "Building tree completed! Took " << build_tree_time << "ms\n";
		std::cout << "Done loading! Read file took: " << read_file_time << "ms\n";
	}
	else {
		std::cout << "Unable to load obj. file: " << filename;
		throw std::runtime_error("Unable to load obj. file: " + filename);
	}


	return scene;
}

vec3 stringToVec3(std::string str) {
	vec3 vec;
	size_t offset = 0;
	vec.x() = stof(getToken(str, offset));
	vec.y() = stof(getToken(str, offset));
	vec.z() = stof(getToken(str, offset));
	return vec;
}

vec3 getVec3(std::ifstream& file) {
	static std::string line;
	getline(file, line);
	return stringToVec3(line);
}

vec4 stringToVec4(std::string str) {
	vec4 vec;
	size_t offset = 0;
	vec.x() = stof(getToken(str, offset));
	vec.y() = stof(getToken(str, offset));
	vec.z() = stof(getToken(str, offset));
	vec.w = 1.0f;
	return vec;
}

vec4 getVec4(std::ifstream& file) {
	static std::string line;
	getline(file, line);
	return stringToVec4(line);
}

float getFloat(std::ifstream& file) {
	static std::string line;
	getline(file, line);
	return stof(line);
}

Scene loadScene(std::string filename) {
	std::ifstream file;
	std::filesystem::path path(filename);

	Scene scene;

	std::string line;

	file.open(path);

	if (file.is_open()) {
		while (!file.eof()) {
			getline(file, line);
			if (line == "GGX") {
				auto material = std::make_shared<GGX>();
				getline(file, line);
				material->name = line;
				material->ambient_color = getVec4(file);
				material->diffuse_color = getVec4(file);
				material->specular_color = getVec4(file);
				material->specular_exp = getFloat(file);
				material->refraction = getFloat(file);
				scene.materials.emplace_back(material);
			}
			else if (line == "Matte") {
				auto material = std::make_shared<Matte>();
				getline(file, line);
				material->name = line;
				material->diffuse_color = getVec4(file);
				scene.materials.emplace_back(material);
			}
			else if (line == "Object") {
				getline(file, line);
				loadobj((path.parent_path() / line).string());
			}
			else if (line == "Sphere") {
				auto sphere = std::make_shared<Sphere>();
				sphere->center = getVec3(file);
				sphere->r = getFloat(file);
				sphere->material = scene.materials[(int) getFloat(file)];
			}
			else if (line == "Skybox") {
				getline(file, line);
				if (line == "Default") {
					auto environment = std::make_shared<DefaultEnvironment>();
					environment->sun_direction = getVec3(file);
					environment->cos_theta_max = getFloat(file);
					environment->sun_intensity = getFloat(file);
					environment->sky_color = getVec4(file);
					environment->p_sun = getFloat(file);
					scene.environment = environment;
				}
				else if (line == "HDR") {
					getline(file, line);
					scene.environment = loadEnvHDR((path.parent_path() / line).string());
				}
			}
		}
		file.close();
	}

	return scene;
}

void writeVec4(std::ofstream& file, vec4 v) {
	file << v.v.x() << " " << v.v.y() << " " << v.z() << " " << v.w << std::endl;
}

//Scene saveScene(std::string filename) {
//    std::ofstream file;
//    std::filesystem::path path(filename);
//
//    Scene scene;
//
//    std::string line;
//
//    file.open(path);
//
//    if (file.is_open()) {
//        while (!file.eof()) {
//            getline(file, line);
//            if (line == "GGX") {
//                auto material = std::make_shared<GGX>();
//                getline(file, line);
//                material->name = line;
//                material->ambient_color = getVec4(file);
//                material->diffuse_color = getVec4(file);
//                material->specular_color = getVec4(file);
//                material->specular_exp = getFloat(file);
//                material->refraction = getFloat(file);
//                scene.materials.emplace_back(material);
//            }
//            else if (line == "Matte") {
//                auto material = std::make_shared<Matte>();
//                getline(file, line);
//                material->name = line;
//                material->diffuse_color = getVec4(file);
//                scene.materials.emplace_back(material);
//            }
//            else if (line == "Object") {
//                getline(file, line);
//                loadobj((path.parent_path() / line).string());
//            }
//            else if (line == "Sphere") {
//                auto sphere = std::make_shared<Sphere>();
//                sphere->center = getVec4(file);
//                sphere->r = getFloat(file);
//                sphere->material = scene.materials[(int)getFloat(file)];
//            }
//            else if (line == "Skybox") {
//                getline(file, line);
//                if (line == "Default") {
//                    auto environment = std::make_shared<DefaultEnvironment>();
//                    environment->sun_direction = getVec4(file);
//                    environment->cos_theta_max = getFloat(file);
//                    environment->sun_intensity = getFloat(file);
//                    environment->sky_color = getVec4(file);
//                    environment->p_sun = getFloat(file);
//                    scene.environment = environment;
//                }
//                else if (line == "HDR") {
//                    getline(file, line);
//                    scene.environment = loadEnvHDR((path.parent_path() / line).string());
//                }
//            }
//        }
//        file.close();
//    }
//}