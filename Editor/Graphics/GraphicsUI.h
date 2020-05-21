#pragma once

#include "Graphics/Shaders/Material.h"
#include "Graphics/Textures/Texture.h"

Material material_dropdown(Material, const char*);
void material_tab(Material*);

Texture texture_dropdown(Texture, const char*);
void texture_tab(Texture*);
