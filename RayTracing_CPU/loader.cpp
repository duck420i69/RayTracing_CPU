#include "loader.h"
#include "accelerator.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


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
        std::shared_ptr<Material> newMTL = std::make_shared<Plastic>();
        while (!file.eof()) {
            std::string str;
            getline(file, str);
            std::stringstream input(str);
            input >> str;
            if (str == "newmtl") {
                if (hasMaterial) {
                    newMTL->diffuse_color = newMTL->diffuse_color * (1.0f / pi);
                    if (!newMTL->isTransparent())
                        newMTL->specular_color = newMTL->specular_color * ((newMTL->specular_exp + 1) / TwoPi);
                    data.emplace_back(newMTL);
                    std::cout << data.size() << "\n";
                    std::cout << "Name: " << newMTL->name << "\n";
                    std::cout << "Diffuse: " << newMTL->diffuse_color.v(0) << " " << newMTL->diffuse_color.v(1) << " " << newMTL->diffuse_color.v(2) << "\n";
                    std::cout << "Specilar: " << newMTL->specular_color.v(0) << " " << newMTL->specular_color.v(1) << " " << newMTL->specular_color.v(2) << "\n";
                    std::cout << "Specualar Exp: " << newMTL->specular_exp << "\n\n";
                }
                hasMaterial = true;
                input >> str;
                if (str.find("Glass") != std::string::npos)
                    newMTL = std::make_shared<ThinGlass>();
                else if (str.find("Chrome") != std::string::npos)
                    newMTL = std::make_shared<Metal>();
                else
                    newMTL = std::make_shared<Plastic>();
                newMTL->name = str;
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
        if (hasMaterial) {
            newMTL->diffuse_color = newMTL->diffuse_color * (1.0f / pi);
            if (!newMTL->isTransparent())
                newMTL->specular_color = newMTL->specular_color * ((newMTL->specular_exp + 1) / TwoPi);
            data.emplace_back(newMTL);
            std::cout << data.size() << "\n";
            std::cout << "Diffuse: " << newMTL->diffuse_color.v(0) << " " << newMTL->diffuse_color.v(1) << " " << newMTL->diffuse_color.v(2) << "\n";
            std::cout << "Specilar: " << newMTL->specular_color.v(0) << " " << newMTL->specular_color.v(1) << " " << newMTL->specular_color.v(2) << "\n";
            std::cout << "Specualar Exp: " << newMTL->specular_exp << "\n\n";
        }
    }
    else {
        std::cout << "Unable to load material. file: " << filename;
        throw "yeet";
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
        throw "yeet";
    }
    std::vector<vec4> env_map;
    float surface_area = 0.0f;
    env_map.resize(width * height);
    for (int i = 0; i < width * height; i++) {
        if (i % width == 0)
            surface_area = (cos(i / width / (float)height * pi) - cos((i / width + 1) / (float)height * pi)) / width;
        env_map[i].v(0) = data[i * 4];
        env_map[i].v(1) = data[i * 4 + 1];
        env_map[i].v(2) = data[i * 4 + 2];
        env_map[i].w = surface_area * env_map[i].v.norm();  // used as probability to choose the pixel
    }
    free(data);
    return std::make_shared<EnvironmentHDR>(std::move(env_map), width, height);
}


std::string getToken(const std::string& str, size_t& offset) {
    auto newOffset = str.find(' ', offset);
    std::string token = str.substr(offset, newOffset - offset);
    offset = ++newOffset;
    return token;
}

Scene loadobj(std::string filename) {
    std::shared_ptr<Object> newObject = std::make_shared<Object>();
    std::deque<std::unique_ptr<BVHNode>> bvh;
    std::vector<vec4> vertices;
    std::vector<texel> vertex;

    Scene scene;

    std::shared_ptr<Matte> default_material = std::make_shared<Matte>();
    default_material->diffuse_color = { 0.256f, 0.256f, 0.256f, 1.0f };

    auto start = std::chrono::system_clock::now();
    auto end = start;

    long long build_tree_time = 0;
    long long read_file_time = 0;

    std::ifstream file;
    std::filesystem::path path(filename);
    path = std::filesystem::current_path() / path;
    file.open(path);

    if (file.is_open()) {
        bool inGroup = false;

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
                str = getToken(str, offset);
                for (auto& mat : scene.materials) {
                    if (mat->name == str) {
                        newObject->material = mat;
                        break;
                    }
                }
            }
            else if (token == "g") {
                if (inGroup) {
                    std::cout << getToken(str, offset) << ": triangles count ";
                    std::cout << newObject->mesh.size() << "\n\n";
                    bvh.emplace_back(std::make_unique<BVHNode>(newObject->bound, scene.objects.size()));
                    bvh.back()->cost = newObject->mesh.size();

                    end = std::chrono::system_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                    read_file_time += duration.count();

                    start = std::chrono::system_clock::now();
                    newObject->buildTree();
                    end = std::chrono::system_clock::now();

                    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                    build_tree_time += duration.count();

                    start = end;

                    if (newObject->material == nullptr) newObject->material = default_material;
                    scene.objects.emplace_back(newObject);
                    newObject = std::make_shared<Object>();
                }
                inGroup = true;
            }
            else if (token == "v") {
                vec4 v;
                for (int i = 0; i < 3; i++) {
                    token = getToken(str, offset);
                    v.v(i) = std::stof(token);
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

                uint32_t v[20], vt[20];
                
                uint8_t i = 0;
                while (offset != 0) {
                    auto token = getToken(str, offset);
                    size_t offset2 = 0;
                    auto newOffset = token.find('/', offset2);
                    std::string tokeni = token.substr(offset2, newOffset - offset2);
                    offset2 = ++newOffset;
                    v[i] = std::stoi(tokeni);

                    newOffset = token.find('/', offset2);
                    tokeni = token.substr(offset2, newOffset - offset2);
                    if (tokeni.empty()) continue;
                    offset2 = ++newOffset;
                    vt[i] = std::stoi(tokeni);
                    hasMaterial = true;
                    i++;
                }
                
                if (!hasMaterial) newObject->mesh.emplace_back(Triangle(vertices[v[0] - 1], vertices[v[1] - 1], vertices[v[2] - 1]));
                else newObject->mesh.emplace_back(Triangle(vertices[v[0] - 1], vertices[v[1] - 1], vertices[v[2] - 1], vertex[vt[0] - 1], vertex[vt[1] - 1], vertex[vt[2] - 1]));
                newObject->bound = Union(newObject->bound, newObject->mesh.back().getBound());
            }

            end = std::chrono::system_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            read_file_time += duration.count();
        }
        bvh.emplace_back(std::make_unique<BVHNode>(newObject->bound, scene.objects.size()));
        bvh.back()->cost = newObject->mesh.size();
        newObject->buildTree();
        if (newObject->material == nullptr) newObject->material = default_material;
        scene.objects.emplace_back(newObject);

        scene.materials.emplace_back(default_material);

        
        auto sphere = std::make_shared<Sphere>();
        sphere->center = vec4(8.0f, 1.0f, 0.0f, 1.0f);
        sphere->r = 1.0f;
        sphere->material = std::make_shared<Mirror>();
        sphere->material->diffuse_color = { 0.9f, 0.9f, 0.9f,1 };
        sphere->material->specular_color = { 1.0f, 1.0f, 1.0f, 1 };
        sphere->material->refraction = 1.5f;
        bvh.emplace_back(std::make_unique<BVHNode>(sphere->getBound(), scene.objects.size()));
        scene.objects.emplace_back(sphere);

        sphere = std::make_shared<Sphere>();
        sphere->center = vec4(5.0f, 7.5f, 2.0f, 1.0f);
        sphere->r = 1.5f;
        sphere->material = std::make_shared<Glass>();
        sphere->material->diffuse_color = { 0.3f, 0.9f, 0.3f,1 };
        sphere->material->specular_color = { 1.0f, 1.0f, 1.0f, 1 };
        sphere->material->refraction = 1.5f;
        bvh.emplace_back(std::make_unique<BVHNode>(sphere->getBound(), scene.objects.size()));
        scene.objects.emplace_back(sphere);

        sphere = std::make_shared<Sphere>();
        sphere->center = vec4(-5.0f, 1.0f, 0.0f, 1.0f);
        sphere->r = 0.5f;
        sphere->material = std::make_shared<Plastic>();
        sphere->material->diffuse_color = { 0.9f, 0.0f, 0.0f, 1 };
        sphere->material->specular_color = { 0.3f, 0.3f, 0.3f, 1 };
        sphere->material->refraction = 1.5f;
        sphere->material->specular_exp = 100.0f;
        bvh.emplace_back(std::make_unique<BVHNode>(sphere->getBound(), scene.objects.size()));
        scene.objects.emplace_back(sphere);

        sphere = std::make_shared<Sphere>();
        sphere->center = vec4(0.0f, 1.0f, 7.0f, 1.0f);
        sphere->r = 0.5f;
        sphere->material = std::make_shared<Plastic>();
        sphere->material->diffuse_color = { 0.0f, 0.0f, 0.9f,1 };
        sphere->material->specular_color = { 0.3f, 0.3f, 0.3f, 1 };
        sphere->material->refraction = 1.5f;
        sphere->material->specular_exp = 100.0f;
        bvh.emplace_back(std::make_unique<BVHNode>(sphere->getBound(), scene.objects.size()));
        scene.objects.emplace_back(sphere);

        sphere = std::make_shared<Sphere>();
        sphere->center = vec4(5.0f, 1.0f, 0.0f, 1.0f);
        sphere->r = 0.5f;
        sphere->material = std::make_shared<Plastic>();
        sphere->material->diffuse_color = { 0.0f, 0.9f, 0.0f,1 };
        sphere->material->specular_color = { 0.3f, 0.3f, 0.3f, 1 };
        sphere->material->refraction = 1.5f;
        sphere->material->specular_exp = 100.0f;
        bvh.emplace_back(std::make_unique<BVHNode>(sphere->getBound(), scene.objects.size()));
        scene.objects.emplace_back(sphere);

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
        end = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        read_file_time += duration.count();

        start = std::chrono::system_clock::now();

        size_t i = 0;
        constructLinearBVH(constructBVH(std::move(bvh), BuildStrat::TOPDOWN), scene.tree, i);

        end = std::chrono::system_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        build_tree_time += duration.count();
        std::cout << "Building tree completed! Took " << build_tree_time << "ms\n";
        std::cout << "Done loading! Read file took: " << read_file_time << "ms\n";
    }
    else {
        std::cout << "Unable to load obj. file: " << filename;
        throw "yeet";
    }


    return scene;
}
