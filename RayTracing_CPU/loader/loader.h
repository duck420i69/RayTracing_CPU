#pragma once

#include "../Scene.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>


struct PixelInfo {
    uint8_t r, g, b, a;
};

std::shared_ptr<EnvironmentHDR> loadEnvHDR(std::string filename);

Scene loadobj(std::string filename);



