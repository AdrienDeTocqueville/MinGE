#pragma once

#include <memory>
#include "Assets/Texture.h"

enum RenderPass
{
	ShadowMap,
	Forward,
	Additive
};


typedef std::shared_ptr<class Material> MaterialRef;

class Material
{
public:
	/// Methods (static)
	static MaterialRef create(std::string name);
	static MaterialRef getDefault();

	/// Methods (public)
	void bind() const;
	bool hasRenderPass(RenderPass pass) const;

	size_t getProperty(const std::string &name) const;

	MaterialRef clone() const;

	template <typename T>
	inline void set(std::string name, T value)
	{
		set(getProperty(name), value);
	}

	template <typename T>
	inline void set(size_t prop, T value)
	{
		memcpy(uniforms.data() + prop, &value, sizeof(T));
	}

private:
	Material(class Program *_program);
	Material(const Material &material);

	class Program *program;
	std::vector<uint8_t> uniforms; // Contains < data > sequenced for each uniform


	static const Material *bound;
	static std::weak_ptr<Material> basic;
};

template<>
void Material::set(size_t prop, Texture *value);
