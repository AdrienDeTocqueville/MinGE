#pragma once

#include <vector>
#include <string>

#include "Core/Asset.h"
#include "Core/UID.h"

#include "Structures/MultiArray.h"
#include "Render/RenderPass.h"


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


	static const void *bound;
};

struct Material: public UID32
{
	Material() {}

	void destroy();
	class Shader *shader() { return materials.get<0>(id())->shader;  }
	const char *uri() const { return *materials.get<1>(id()); }
	bool is_valid() { return id() && *materials.get<2>(id()) == gen(); }

	void define(const std::vector<std::string> &macros);
	void define(const std::string& macro);
	void undef(const std::string& macro);
	bool defined(const std::string& macro) const;

	inline bool has_pass(RenderPass::Type pass) const;
	size_t get_location(const std::string &name) const;

	template <typename T>
	inline void set(const std::string &name, const T &value);

	template <typename T>
	inline void set(size_t location, const T &value);

	template <typename T>
	inline void set(const std::string &name, const T *values, size_t num);

	template <typename T>
	inline void set(size_t location, const T *values, size_t num);


	static Material load(const char *URI);
	static Material get(uint32_t i);
	static void clear();

	static void reload(class Shader *shader);
	static Material copy(Material src);

	static const Material none;
	static multi_array_t<material_t, char*, uint8_t> materials;

	static const asset_type_t type;

private:
	Material(uint32_t i, uint32_t g): UID32(i, g) {}
};
