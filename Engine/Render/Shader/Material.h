#pragma once

#include <vector>
#include <string>

#include "Core/AssetDatabase.h"

#include "Structures/ArrayList.h"
#include "Render/Shader/Shader.h"

struct Material: public UID32
{
	struct material_t : object_data_t
	{
		inline bool has_pass(RenderPass::Type pass) const;
		void bind(RenderPass::Type pass) const;

		inline void update_variant(uint32_t hash);
		inline void sync_uniforms();

		uint32_t variant_hash;
		uint32_t variant_idx;
		Shader shader;
		std::vector<uint8_t> uniforms; // Contains < data > sequenced for each uniform

		static const void *bound;
	};

	Material() {}
	Material(UID32 uid): UID32(uid) {}

	void destroy();
	bool is_valid() const { return valid() && materials[id()].valid(gen()); }

	Shader shader() const { return materials[id()].shader;  }

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


	static Material create(Material src);
	static Material create(Shader shader, uint32_t variant = 0);
	static Material get(uint32_t i) { return UID32(i, materials[i].gen); }

	static array_list_t<material_t> materials;

	static const struct object_type_t type;
	//static const struct asset_importer_t importer;
};
