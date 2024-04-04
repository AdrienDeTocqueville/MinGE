#pragma once

#include <string.h>
#include <string>
#include <memory>

#include "Render/Shader/Material.h"
#include "Render/Shader/Shader.inl"

bool Material::has_pass(RenderPass::Type pass) const
{
	material_t &m = materials[id()];
	return m.shader.get_data().passes[pass].exists;
}

template <typename T>
inline void Material::set(const std::string &name, const T &value)
{
	size_t loc = get_location(name);
	if (loc != (size_t)-1)
		set(loc, value);
}

template <typename T>
inline void Material::set(size_t location, const T &value)
{
	material_t &m = materials[id()];
	memcpy(m.uniforms.data() + location, &value, sizeof(T));
}

template <typename T>
inline void Material::set(const std::string &name, const T *values, size_t num)
{
	size_t loc = get_location(name);
	if (loc != (size_t)-1)
		set(loc, values, num);
}

template <typename T>
inline void Material::set(size_t location, const T *values, size_t num)
{
	material_t &m = materials[id()];
	memcpy(m.uniforms.data() + location, values, sizeof(T) * num);
}



inline bool Material::material_t::has_pass(RenderPass::Type pass) const
{
	return shader.get_data().passes[pass].exists;
}


inline void Material::material_t::update_variant(uint32_t hash)
{
	variant_hash = hash;
	variant_idx = shader.get_data().get_variant(hash);
	sync_uniforms();
}

inline void Material::material_t::sync_uniforms()
{
	size_t old_size = uniforms.size();
	uniforms.resize(shader.get_data().uniform_offset);
	memset(uniforms.data() + old_size, 0, (uniforms.size() - old_size) * sizeof(uint8_t));
}
