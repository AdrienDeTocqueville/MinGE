#pragma once

#include <Render/Mesh/Mesh.h>
#include <Render/Shaders/Material.h>
#include <Render/Textures/Texture.h>

bool mesh_dropdown(const char*, Mesh*);
void mesh_tab(Mesh*);

bool material_dropdown(const char*, Material*);
void material_tab(Material*);

bool texture_dropdown(const char*, Texture*);
void texture_tab(Texture*);
