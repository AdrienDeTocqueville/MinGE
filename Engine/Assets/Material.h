#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <memory>
#include <vector>

#include "Renderer/structs.h"

typedef std::shared_ptr<class Material> MaterialRef;

class Material
{
public:
	/// Methods (static)
	static MaterialRef create(std::string name);
	static MaterialRef getDefault();
	static Material *get(uint32_t id) { return materials[id]; }


	/// Methods (public)
	void bind(RenderPass::Type pass) const;
	bool has_pass(RenderPass::Type pass) const;
	void reload();

	void define(const std::vector<std::string> &macros);
	void define(const std::string& macro);
	void undef(const std::string& macro);
	bool ifdef(const std::string& macro) const;

	inline uint32_t get_id() const { return id; }
	size_t get_location(const std::string &name) const;

	MaterialRef clone() const;

	template <typename T>
	inline void set(std::string name, T value)
	{
		size_t loc = get_location(name);
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
		size_t loc = get_location(name);
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
	Material(class Shader *_shader);
	Material(const Material &material);

	void update_variant();
	void compute_id(size_t first);
	void sync_uniforms();

	uint32_t variant_hash;
	uint32_t variant_idx;
	class Shader *shader;
	std::vector<uint8_t> uniforms; // Contains < data > sequenced for each uniform

	uint32_t id;


	static const Material *bound;
	static std::weak_ptr<Material> standard;
	static std::vector<Material*> materials;

	friend struct SetupView;
};

template<>
void Material::set(size_t location, const class Texture *value);
template<>
void Material::set(size_t location, class Texture *value);
