#pragma once

#include <memory>
#include "Assets/Texture.h"

enum RenderPass
{
	ShadowMap,
	Forward,
	Additive,
	Count
};


typedef std::shared_ptr<class Material> MaterialRef;

class Material
{
public:
	/// Methods (static)
	static MaterialRef create(std::string name);
	static MaterialRef getDefault();
	static MaterialRef get(uint32_t id);

	/// Methods (public)
	void bind(RenderPass pass = RenderPass::Forward) const; // TODO: remove default value
	bool hasRenderPass(RenderPass pass) const;

	inline uint32_t getId() const { return id; }
	size_t getLocation(const std::string &name) const;

	MaterialRef clone() const;

	template <typename T>
	inline void set(std::string name, T value)
	{
		set(getLocation(name), value);
	}

	template <typename T>
	inline void set(size_t location, T value)
	{
		memcpy(uniforms.data() + location, &value, sizeof(T));
	}

	~Material();

private:
	Material(class Program *_program);
	Material(const Material &material);

	class Program *program;
	std::vector<uint8_t> uniforms; // Contains < data > sequenced for each uniform

	uint32_t id;


	static const Material *bound;
	static std::weak_ptr<Material> basic;
	static std::vector<std::weak_ptr<Material>> materials;
};

template<>
void Material::set(size_t location, Texture *value);
