#pragma once

#include <Render/Shaders/Material.h>
#include <Render/Textures/Texture.h>

Material material_dropdown(Material, const char*);
void material_tab(Material*);

Texture texture_dropdown(Texture, const char*);
void texture_tab(Texture*);
