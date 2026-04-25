#include "MaterialBuilder.h"

#include <iostream>



MaterialBuilder::MaterialBuilder() {
	reset();
}

MaterialBuilder& MaterialBuilder::setName(std::string name) {
	m_name = name;
	return *this;
}

MaterialBuilder& MaterialBuilder::setType(MaterialType type) {
	m_type = type;
	return *this;
}

MaterialBuilder& MaterialBuilder::setAmbientColor(const Color& color) {
	m_ambient_color = color;
	return *this;
}

MaterialBuilder& MaterialBuilder::setSpecularColor(const Color& color) {
	m_specular_color = color;
	return *this;
}

MaterialBuilder& MaterialBuilder::setDiffuseColor(const Color& color) {
	m_diffuse_color = color;
	return *this;
}

MaterialBuilder& MaterialBuilder::setRefraction(float ior) {
	m_refraction = ior;
	return *this;
}

MaterialBuilder& MaterialBuilder::setShininess(float shininess) {
	m_specular_exp = shininess;
	return *this;
}

MaterialBuilder& MaterialBuilder::setTexture(int width, int height, std::vector<Color>&& texture) {
	m_width = width; m_height = height; m_texture_map = texture;
	return *this;
}

std::shared_ptr<Material> MaterialBuilder::build() {
	std::shared_ptr<Material> newMaterial;

	if (!m_type.has_value()) {
		m_type = inferType();
	}

	switch (m_type.value()) {
		case MaterialType::MATTE:
		{
			newMaterial = std::make_shared<Matte>();
			break;
		}
		case MaterialType::METAL:
		{
			newMaterial = std::make_shared<Matte>();
			break;
		}
		case MaterialType::PLASTIC:
		{
			newMaterial = std::make_shared<Plastic>();
			newMaterial->specular_color = m_specular_color * (m_specular_exp + 1);
			break;
		}
		case MaterialType::GGX:
		{
			newMaterial = std::make_shared<GGX>(sqrt(2 / (m_specular_exp + 2)));
			break;
		}
		case MaterialType::THIN_GLASS:
		{
			newMaterial = std::make_shared<ThinGlass>();
			break;
		}
			
		default:
			break;
	}

	newMaterial->name = m_name;
	newMaterial->ambient_color = m_ambient_color;
	newMaterial->specular_color = m_specular_color;
	newMaterial->diffuse_color = m_diffuse_color / pi;
	newMaterial->width = m_width;
	newMaterial->height = m_height;
	newMaterial->texture_map = std::move(m_texture_map);
	newMaterial->refraction = m_refraction;
	newMaterial->specular_exp = m_specular_exp;

	reset();

	return newMaterial;
}

void MaterialBuilder::reset() {
	m_name = "";
	m_diffuse_color = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_specular_color = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_diffuse_color = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_width = 0;
	m_height = 0;
	m_refraction = 1.5f;
	m_specular_exp = 150.0f;
	m_texture_map.clear();
}

MaterialType MaterialBuilder::inferType() {
	if (m_name.find("Glass") != std::string::npos)
		return MaterialType::THIN_GLASS;
	else if (m_name.find("Metal") != std::string::npos)
		return MaterialType::METAL;
	else if (m_name.find("Matte") != std::string::npos)
		return MaterialType::MATTE;
	else if (m_name.find("Plastic") != std::string::npos)
		return MaterialType::PLASTIC;
	else
		return MaterialType::GGX;
}
