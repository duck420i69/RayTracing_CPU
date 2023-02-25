#pragma once

#include "Shape.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <sstream>

struct PixelInfo {
    uint8_t r, g, b, a;
};

class Tga
{
private:
	std::vector<std::uint8_t> Pixels;
	bool ImageCompressed;
	std::uint32_t width, height, size, BitsPerPixel;

public:
	Tga(std::filesystem::path FilePath);
	std::vector<std::uint8_t> GetPixels() { return this->Pixels; }
	std::uint32_t GetWidth() const { return this->width; }
	std::uint32_t GetHeight() const { return this->height; }
	bool HasAlphaChannel() { return BitsPerPixel == 32; }
};


Scene loadobj(std::string filename);



