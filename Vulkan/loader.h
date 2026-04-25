#pragma once

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>

#include "VulkanObject.h"


struct PixelInfo {
    uint8_t r, g, b, a;
};


Mesh loadobj(std::string filename);



