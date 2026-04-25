#include "loader.h"
#include <algorithm>


std::string getToken(const std::string& str, size_t& offset, char lim = ' ') {
    auto newOffset = str.find(lim, offset);
    std::string token = str.substr(offset, newOffset - offset);
    offset = ++newOffset;
    return token;
}

Mesh loadobj(std::string filename) {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> vertex;
    std::vector<glm::vec3> normal;

    Mesh mesh;

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

            if (token == "g") {

            }
            else if (token == "v") {
                glm::vec3 v;
                token = getToken(str, offset);
                v.x = std::stof(token);
                token = getToken(str, offset);
                v.y = std::stof(token);
                token = getToken(str, offset);
                v.z = std::stof(token);
                vertices.emplace_back(v);
            }
            else if (token == "vn") {
                glm::vec3 v;
                token = getToken(str, offset);
                v.x = std::stof(token);
                token = getToken(str, offset);
                v.y = std::stof(token);
                token = getToken(str, offset);
                v.z = std::stof(token);
                normal.emplace_back(v);
            }
            else if (token == "vt") {
                glm::vec2 temp;
                token = getToken(str, offset);
                temp.x = std::stof(token);
                token = getToken(str, offset);
                temp.y = std::stof(token);
                vertex.emplace_back(temp);
            }
            else if (token == "f") {
                uint32_t v[20]{ 0 }, vt[20]{ 0 }, vn[20]{ 0 };
                
                uint8_t i = -1;
                while (offset != 0) {
                    i++;
                    auto token = getToken(str, offset);
                    size_t offset2 = 0;
                    v[i] = std::stoi(getToken(token, offset2, '/'));

                    auto tokeni = getToken(token, offset2, '/');
                    if (!tokeni.empty()) {
                        vt[i] = std::stoi(tokeni);
                    }

                    tokeni = getToken(token, offset2);
                    if (!tokeni.empty()) {
                        vn[i] = std::stoi(tokeni);
                    }
                    // std::stoi(tokeni);
                }
                
                for (int i = 0; i < 3; i++) {
                    Vertex vertex;
                    vertex.position = vertices[v[i] - 1];
                    if (normal.size() > 0) {
                        vertex.normal = normal[vn[i] - 1];
                        vertex.color = normal[vn[i] - 1];
                    }
                    else {
                        glm::vec3 tnormal = glm::cross(vertices[v[1] - 1] - vertices[v[0] - 1], vertices[v[2] - 1] - vertices[v[0] - 1]);
                        vertex.normal = glm::abs(glm::normalize(tnormal));
                        vertex.color = glm::abs(glm::normalize(tnormal));
                    }
                    mesh.vertices.push_back(vertex);
                }
            }

            end = std::chrono::system_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            read_file_time += duration.count();
        }
    }
    else {
        std::cout << "Unable to load obj. file: " << filename;
        throw "yeet";
    }

    return mesh;
}
