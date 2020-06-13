#pragma once

#include <string.h>
#include <string>
#include <memory>

#include "Render/Shaders/Material.h"
#include "Render/Shaders/Shader.inl"

bool Material::has_pass(RenderPass::Type pass) const
{
	material_t *m = materials.get<0>(id());
	return m->shader->passes[pass].exists;
}

template <typename T>
inline void Material::set(const std::string &name, T value)
{
	size_t loc = get_location(name);
	if (loc != (size_t)-1)
		set(loc, value);
}

template <typename T>
inline void Material::set(size_t location, T value)
{
	material_t *m = materials.get<0>(id());
	memcpy(m->uniforms.data() + location, &value, sizeof(T));
}

template <typename T>
inline void Material::set(const std::string &name, T *values, size_t num)
{
	size_t loc = get_location(name);
	if (loc != (size_t)-1)
		set(loc, values, num);
}

template <typename T>
inline void Material::set(size_t location, T *values, size_t num)
{
	material_t *m = materials.get<0>(id());
	memcpy(m->uniforms.data() + location, values, sizeof(T) * num);
}



inline bool material_t::has_pass(RenderPass::Type pass) const
{
	return shader->passes[pass].exists;
}


inline void material_t::update_variant(uint32_t hash)
{
	variant_hash = hash;
	variant_idx = shader->get_variant(hash);
	sync_uniforms();
}

inline void material_t::sync_uniforms()
{
	size_t old_size = uniforms.size();
	uniforms.resize(shader->uniform_offset);
	memset(uniforms.data() + old_size, 0, (uniforms.size() - old_size) * sizeof(uint8_t));
}
