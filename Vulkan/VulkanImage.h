#pragma once

#include "vulkan.h"


class Image {
public:
    Image(const char* filename);
    Image(const Image& other);
    Image& operator=(const Image& other);
    ~Image();
private:
    uint32_t* data;
    uint32_t width, height;
};