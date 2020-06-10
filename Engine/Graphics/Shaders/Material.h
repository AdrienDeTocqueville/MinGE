#pragma once

#include <vector>

#include "Core/UID.h"
#include "Structures/MultiArray.h"
#include "Graphics/RenderPass.h"


struct material_t
{
	/// Methods (public)
	inline bool has_pass(RenderPass::Type pass) const;
	void bind(RenderPass::Type pass) const;

	inline void update_variant(uint32_t hash);
	inline void sync_uniforms();

	uint32_t variant_hash;
	uint32_t variant_idx;
	class Shader *shader;
	std::vector<uint8_t> uniforms; // Contains < data > sequenced for each uniform


	static const material_t *bound;
};

struct Material: public UID32
{
	Material() {}

	void destroy();
	class Shader *shader() { return materials.get<0>(id())->shader;  }
	bool is_valid() { return id() && *materials.get<1>(id()) == gen(); }

	void define(const std::vector<std::string> &macros);
	void define(const std::string& macro);
	void undef(const std::string& macro);
	bool defined(const std::string& macro) const;

	inline bool has_pass(RenderPass::Type pass) const;
	size_t get_location(const std::string &name) const;

	template <typename T>
	inline void set(const std::string &name, T value);

	template <typename T>
	inline void set(size_t location, T value);

	template <typename T>
	inline void set(const std::string &name, T *values, size_t num);

	template <typename T>
	inline void set(size_t location, T *values, size_t num);


	static Material create(class Shader *shader);
	static Material copy(Material src);
	static Material get(uint32_t i);
	static void reload(class Shader *shader);
	static void clear();

	static const Material none;
	static multi_array_t<material_t, uint8_t> materials;

private:
	Material(uint32_t i, uint32_t g): UID32(i, g) {}
};
