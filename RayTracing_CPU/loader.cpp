#include "loader.h"
#include "accelerator.h"
#include <iostream>
#include <algorithm>
#include <chrono>


Tga::Tga(std::filesystem::path FilePath)
{
    std::fstream hFile(FilePath, std::ios::in | std::ios::binary);
    if (!hFile.is_open()) { throw std::invalid_argument("File Not Found."); }

    std::uint8_t Header[18] = { 0 };
    std::vector<std::uint8_t> ImageData;
    static std::uint8_t DeCompressed[12] = { 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
    static std::uint8_t IsCompressed[12] = { 0x0, 0x0, 0xA, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

    hFile.read(reinterpret_cast<char*>(&Header), sizeof(Header));

    if (!std::memcmp(DeCompressed, &Header, sizeof(DeCompressed)))
    {
        BitsPerPixel = Header[16];
        width = Header[13] * 256 + Header[12];
        height = Header[15] * 256 + Header[14];
        size = ((width * BitsPerPixel + 31) / 32) * 4 * height;

        if ((BitsPerPixel != 24) && (BitsPerPixel != 32))
        {
            hFile.close();
            throw std::invalid_argument("Invalid File Format. Required: 24 or 32 Bit Image.");
        }

        ImageData.resize(size);
        ImageCompressed = false;
        hFile.read(reinterpret_cast<char*>(ImageData.data()), size);
    }
    else if (!std::memcmp(IsCompressed, &Header, sizeof(IsCompressed)))
    {
        BitsPerPixel = Header[16];
        width = Header[13] * 256 + Header[12];
        height = Header[15] * 256 + Header[14];
        size = ((width * BitsPerPixel + 31) / 32) * 4 * height;

        if ((BitsPerPixel != 24) && (BitsPerPixel != 32))
        {
            hFile.close();
            throw std::invalid_argument("Invalid File Format. Required: 24 or 32 Bit Image.");
        }

        PixelInfo Pixel = { 0 };
        int CurrentByte = 0;
        std::size_t CurrentPixel = 0;
        ImageCompressed = true;
        std::uint8_t ChunkHeader = { 0 };
        int BytesPerPixel = (BitsPerPixel / 8);
        ImageData.resize(width * height * sizeof(PixelInfo));

        do
        {
            hFile.read(reinterpret_cast<char*>(&ChunkHeader), sizeof(ChunkHeader));

            if (ChunkHeader < 128)
            {
                ++ChunkHeader;
                for (int I = 0; I < ChunkHeader; ++I, ++CurrentPixel)
                {
                    hFile.read(reinterpret_cast<char*>(&Pixel), BytesPerPixel);

                    ImageData[CurrentByte++] = Pixel.b;
                    ImageData[CurrentByte++] = Pixel.g;
                    ImageData[CurrentByte++] = Pixel.r;
                    if (BitsPerPixel > 24) ImageData[CurrentByte++] = Pixel.a;
                }
            }
            else
            {
                ChunkHeader -= 127;
                hFile.read(reinterpret_cast<char*>(&Pixel), BytesPerPixel);

                for (int I = 0; I < ChunkHeader; ++I, ++CurrentPixel)
                {
                    ImageData[CurrentByte++] = Pixel.b;
                    ImageData[CurrentByte++] = Pixel.g;
                    ImageData[CurrentByte++] = Pixel.r;
                    if (BitsPerPixel > 24) ImageData[CurrentByte++] = Pixel.a;
                }
            }
        } while (CurrentPixel < (width * height));
    }
    else
    {
        hFile.close();
        throw std::invalid_argument("Invalid File Format. Required: 24 or 32 Bit TGA File.");
    }

    hFile.close();
    this->Pixels = ImageData;
}

std::vector<std::shared_ptr<Material>> loadMTL(std::filesystem::path filename) {
    std::vector<std::shared_ptr<Material>> data;
    std::ifstream file;
    bool hasMaterial = false;
    file.open(filename);
    if (file.is_open()) {
        auto newMTL = std::make_shared<Plastic>();
        while (!file.eof()) {
            std::string str;
            getline(file, str);
            std::stringstream input(str);
            input >> str;
            if (str == "newmtl") {
                if (hasMaterial) {
                    newMTL->diffuse_color * (1.0f / pi);
                    data.emplace_back(newMTL);
                    newMTL = std::make_shared<Plastic>();
                    std::cout << data.size() << "\n\n";
                }
                hasMaterial = true;
                input >> newMTL->name;
            }
            else if (str == "Ka") {
                for (int i = 0; i < 3; i++) input >> newMTL->ambient_color.v(i);
            }
            else if (str == "Kd") {
                for (int i = 0; i < 3; i++) input >> newMTL->diffuse_color.v(i);
            }
            else if (str == "Ks") {
                for (int i = 0; i < 3; i++) input >> newMTL->specular_color.v(i);
            }
            else if (str == "Ni") {
                input >> newMTL->refraction;
            }
            else if (str == "Ns") {
                input >> newMTL->specular_exp;
            }
            else if (str == "map_Kd") {
                input >> str;
                Tga newMaterial(filename.parent_path() / str);
                if (newMaterial.HasAlphaChannel()) {
                    newMTL->texture_map.resize(newMaterial.GetWidth() * newMaterial.GetHeight());
                    std::vector<uint8_t> pixels = newMaterial.GetPixels();
                    newMTL->width = newMaterial.GetWidth();
                    newMTL->height = newMaterial.GetHeight();
                    for (long i = 0; i < newMTL->texture_map.size(); i++) {
                        newMTL->texture_map[i].v(0) = pixels[i * 4 + 2] / 256.0f;
                        newMTL->texture_map[i].v(1) = pixels[i * 4 + 1] / 256.0f;
                        newMTL->texture_map[i].v(2) = pixels[i * 4 + 0] / 256.0f;
                        newMTL->texture_map[i].w = pixels[i * 4 + 3] / 256.0f;
                    }
                }
                else {
                    newMTL->texture_map.resize(newMaterial.GetWidth() * newMaterial.GetHeight());
                    std::vector<uint8_t> pixels = newMaterial.GetPixels();
                    newMTL->width = newMaterial.GetWidth();
                    newMTL->height = newMaterial.GetHeight();
                    for (long i = 0; i < newMTL->texture_map.size(); i++) {
                        newMTL->texture_map[i].v(0) = pixels[i * 3 + 2] / 256.0f;
                        newMTL->texture_map[i].v(1) = pixels[i * 3 + 1] / 256.0f;
                        newMTL->texture_map[i].v(2) = pixels[i * 3 + 0] / 256.0f;
                        newMTL->texture_map[i].w = 1.0f;
                    }
                }
            }
        }
        data.emplace_back(newMTL);
    }
    else {
        std::cout << "Unable to load material. file: " << filename;
        throw "yeet";
    }
    return data;
}

Scene loadobj(std::string filename) {
    std::shared_ptr<Object> newObject = std::make_shared<Object>();
    Scene scene;
    std::deque<std::unique_ptr<BVHNode>> bvh;
    std::vector<vec4> vertices;
    std::vector<texel> vertex;

    std::shared_ptr<Matte> default_material = std::make_shared<Matte>();
    default_material->diffuse_color = { 0.256f, 0.256f, 0.256f, 1.0f };

    std::ifstream file;
    std::filesystem::path path(filename);
    path = std::filesystem::current_path() / path;
    file.open(path);
    if (file.is_open()) {
        auto start = std::chrono::system_clock::now();

        bool hasMaterial = false;
        bool hasNormal = false;
        bool inGroup = false;

        while (!file.eof()) {
            std::string str;
            getline(file, str);
            std::stringstream input(str);
            input >> str;
            if (str == "mtllib") {
                input >> str;
                scene.materials = loadMTL(path.parent_path() / str);
            }
            else if (str == "usemtl") {
                input >> str;
                for (auto& mat : scene.materials) {
                    if (mat->name == str) {
                        newObject->material = mat;
                        break;
                    }
                }
            }
            else if (str == "g") {
                if (inGroup) {
                    bvh.emplace_back(std::make_unique<BVHNode>(newObject->bound, scene.objects.size()));
                    bvh.back()->cost = newObject->mesh.size();
                    newObject->buildTree();
                    if (newObject->material == nullptr) newObject->material = default_material;
                    scene.objects.emplace_back(newObject);
                    newObject = std::make_shared<Object>();
                }
                inGroup = true;
            }
            else if (str == "v") {
                vec4 v;
                input >> v.v(0) >> v.v(1) >> v.v(2); v.w = 1.0f;
                vertices.emplace_back(v);
            }
            else if (str == "vt") {
                texel temp;
                input >> temp.x >> temp.y;
                vertex.emplace_back(temp);
                hasMaterial = true;
            }
            else if (str == "vn") {
                hasNormal = true;
            }
            else if (str == "f") {
                char temp;
                int index;
                int v1, v2, v3, vt1, vt2, vt3, trash;
                if (!hasMaterial && !hasNormal)
                    input >> v1 >> v2 >> v3;
                if (hasMaterial && !hasNormal) {
                    input >> v1 >> temp >> vt1;
                    input >> v2 >> temp >> vt2;
                    input >> v3 >> temp >> vt3;
                }
                if (!hasMaterial && hasNormal) {
                    input >> v1 >> temp >> temp >> trash;
                    input >> v2 >> temp >> temp >> trash;
                    input >> v3 >> temp >> temp >> trash;
                }
                if (hasMaterial && hasNormal) {
                    input >> v1 >> temp >> vt1 >> temp >> trash;
                    input >> v2 >> temp >> vt2 >> temp >> trash;
                    input >> v3 >> temp >> vt3 >> temp >> trash;
                }
                if (!hasMaterial) newObject->mesh.emplace_back(Triangle(vertices[v1 - 1], vertices[v2 - 1], vertices[v3 - 1]));
                else newObject->mesh.emplace_back(Triangle(vertices[v1 - 1], vertices[v2 - 1], vertices[v3 - 1], vertex[vt1 - 1], vertex[vt2 - 1], vertex[vt3 - 1]));
                newObject->bound = Union(newObject->bound, newObject->mesh.back().getBound());
            }
        }
        bvh.emplace_back(std::make_unique<BVHNode>(newObject->bound, scene.objects.size()));
        bvh.back()->cost = newObject->mesh.size();
        newObject->buildTree();
        if (newObject->material == nullptr) newObject->material = default_material;
        scene.objects.emplace_back(newObject);

        scene.materials.emplace_back(default_material);

        
        //auto sphere = std::make_shared<Sphere>();
        //sphere->center = vec4(8.0f, 1.0f, 0.0f, 1.0f);
        //sphere->r = 1.0f;
        //sphere->material = std::make_shared<Mirror>();
        //sphere->material->diffuse_color = { 0.9f, 0.9f, 0.9f,1 };
        //sphere->material->specular_color = { 1.0f, 1.0f, 1.0f, 1 };
        //sphere->material->refraction = 1.5f;
        //bvh.emplace_back(std::make_unique<BVHNode>(sphere->getBound(), scene.objects.size()));
        //scene.objects.emplace_back(sphere);

        /*sphere = std::make_shared<Sphere>();
        sphere->center = vec4(700.0f, 150.5f, 0.0f, 1.0f);
        sphere->r = 75.f;
        sphere->material = std::make_shared<Glass>();
        sphere->material->diffuse_color = { 0.3f, 0.9f, 0.3f,1 };
        sphere->material->specular_color = { 1.0f, 1.0f, 1.0f, 1 };
        sphere->material->refraction = 1.5f;
        bvh.emplace_back(std::make_unique<BVHNode>(sphere->getBound(), scene.objects.size()));
        scene.objects.emplace_back(sphere);*/

        //sphere = std::make_shared<Sphere>();
        //sphere->center = vec4(7.0f, 7.0f, -1.0f, 1.0f);
        //sphere->r = 2.0f;
        //sphere->material = scene.materials[0];
        //bvh.emplace_back(std::make_unique<BVHNode>(sphere->getBound(), scene.objects.size()));
        //scene.objects.emplace_back(sphere);

        //sphere = std::make_shared<Sphere>();
        //sphere->center = vec4(3.0f, 1.2f, 2.2f, 1.0f);
        //sphere->r = 1.2f;
        //sphere->material = std::make_shared<Metal>();
        //sphere->material->diffuse_color = { 0.3f, 0.3f, 0.3f,1 };
        //sphere->material->specular_color = { 0.9f, 0.9f, 0.9f, 1 };
        //sphere->material->refraction = 1.5f;
        //sphere->material->specular_exp = 10.0f;
        //bvh.emplace_back(std::make_unique<BVHNode>(sphere->getBound(), scene.objects.size()));
        //scene.objects.emplace_back(sphere);
        

        file.close();
        auto end = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Done loading! Start building tree. Took " << duration.count() << "ms\n";



        start = std::chrono::system_clock::now();

        std::sort(bvh.begin(), bvh.end(), [](const std::unique_ptr<BVHNode>& b1, const std::unique_ptr<BVHNode>& b2) -> bool {
            return b1->centroid(0) < b2->centroid(0);
            });

        size_t i = 0;
        constructLinearBVH(constructBVH(std::move(bvh)), scene.tree, i);

        end = std::chrono::system_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Building tree completed! Took " << duration.count() << "ms\n";
    }
    else {
        std::cout << "Unable to load obj. file: " << filename;
        throw "yeet";
    }


    return scene;
}
