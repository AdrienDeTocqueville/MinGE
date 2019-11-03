#pragma once

#include <memory>
#include "Assets/Texture.h"
#include "Assets/Program.h"
#include "Renderer/UBO.h"

class Material;
typedef std::shared_ptr<Material> MaterialRef;

class Material
{
	struct Property
	{
		GLuint type;
		GLuint location;
		size_t offset;
	};

public:
	/// Methods (static)
	static MaterialRef create(std::string name);
	static MaterialRef getDefault();

	/// Methods (public)
	MaterialRef clone() const;
	void bind() const;

	template <typename T>
	inline void set(std::string name, T value)
	{
		auto it = property_names.find(name);
		if (it != property_names.end())
			set(it->second, value);
		else
			std::cout << "Unknown uniform: " << name << std::endl;
	}

	template <typename T>
	inline void set(uint32_t prop, T value)
	{
		memcpy(uniforms.data() + properties[prop].offset, &value, sizeof(T));
	}

private:
	Material(Program *_program);

	Program *program;

	std::map<std::string, uint32_t> property_names;
	std::vector<Property> properties;
	std::vector<uint8_t> uniforms;

	static std::weak_ptr<Material> basic;
};

template<>
void Material::set(uint32_t prop, Texture *value);
