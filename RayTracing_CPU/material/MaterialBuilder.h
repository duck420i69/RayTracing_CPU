#pragma once

#include "Material.h"

#include "Metal.h"
#include "Matte.h"
#include "GGX.h"

#include <optional>


class MaterialBuilder {
public:
	MaterialBuilder();

	MaterialBuilder& setName(std::string name);
	MaterialBuilder& setType(MaterialType type);
	MaterialBuilder& setAmbientColor(const Color& color);
	MaterialBuilder& setSpecularColor(const Color& color);
	MaterialBuilder& setDiffuseColor(const Color& color);
	MaterialBuilder& setRefraction(float ior);
	MaterialBuilder& setShininess(float shininess);
	MaterialBuilder& setTexture(int width, int height, std::vector<Color>&& texture);
	std::shared_ptr<Material> build();
	void reset();
private:
	std::string m_name;
	std::optional<MaterialType> m_type;

	Color m_ambient_color;
	Color m_specular_color;
	Color m_diffuse_color;
	float m_refraction, m_metalic, m_specular_exp;
	
	int m_width, m_height;
	std::vector<Color> m_texture_map;

	MaterialType inferType();
};