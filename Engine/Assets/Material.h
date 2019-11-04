#pragma once

#include <memory>
#include "Assets/Texture.h"
#include "Assets/Program.h"
#include "Renderer/UBO.h"

typedef std::shared_ptr<class Material> MaterialRef;

class Material
{
	struct Property
	{
		GLuint location, type;
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
	void set(std::string name, T value)
	{
		auto it = program->uniforms.find(name);
		if (it != program->uniforms.end())
		{
			for (int i(0); i < properties.size(); i++)
				if (properties[i].location == it->second.location)
					set(i, value);
		}
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
	Material(const Material &material);

	Program *program;

	std::vector<Property> builtin_props, properties;
	std::vector<uint8_t> uniforms;

	static const Material *bound;
	static std::weak_ptr<Material> basic;
};

template<>
void Material::set(uint32_t prop, Texture *value);
