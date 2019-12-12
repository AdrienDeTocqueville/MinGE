#pragma once

#include <memory>
#include "Assets/Texture.h"

struct RenderPass
{
	enum Type {
		ShadowMap,
		Forward,
		Additive,
		Skybox,
		Count
	};
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
	void bind(RenderPass::Type pass) const;
	bool hasRenderPass(RenderPass::Type pass) const;
	void reload();

	inline uint32_t getId() const { return id; }
	size_t getLocation(const std::string &name) const;

	MaterialRef clone() const;

	template <typename T>
	inline void set(std::string name, T value)
	{
		size_t loc = getLocation(name);
		if (loc != (size_t)-1)
			set(loc, value);
	}

	template <typename T>
	inline void set(size_t location, T value)
	{
		memcpy(uniforms.data() + location, &value, sizeof(T));
	}

	template <typename T>
	inline void set(std::string name, T *values, size_t num)
	{
		size_t loc = getLocation(name);
		if (loc != (size_t)-1)
			set(loc, values, num);
	}

	template <typename T>
	inline void set(size_t location, T *values, size_t num)
	{
		memcpy(uniforms.data() + location, values, sizeof(T) * num);
	}

	~Material();

private:
	Material(class Program *_program);
	Material(const Material &material);

	void load_uniforms();

	class Program *program;
	std::vector<uint8_t> uniforms; // Contains < data > sequenced for each uniform

	uint32_t id;


	static const Material *bound;
	static std::weak_ptr<Material> basic;
	static std::vector<std::weak_ptr<Material>> materials;
};

template<>
void Material::set(size_t location, Texture *value);
