#pragma once

#include <memory>
#include "Assets/Texture.h"
#include "Assets/Program.h"
#include "Renderer/UBO.h"

class Material;
typedef std::shared_ptr<Material> MaterialRef;

class Material
{
public:
	/// Methods (static)
	static MaterialRef create(Program *program);
	static MaterialRef getDefault();

	/// Methods (public)
	MaterialRef clone() const;
	void bind() const;

	template <typename T>
	void set(std::string name, T value)
	{
		auto it = uniforms.find(name);
		if (it != uniforms.end())
			memcpy(it->second, &value, sizeof(T));
		else
			std::cout << "Unknown uniform" << std::endl;
	}

	~Material();

private:
	Material(Program *_program);

	Program *program;
	std::vector<UBO> ubos;
	std::map<std::string, uint8_t*> uniforms;

	static std::weak_ptr<Material> basic;
};

template<>
void Material::set(std::string name, Texture *value);
