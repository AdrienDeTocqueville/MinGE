#include "Editor.h"

#include "Graphics/GraphicsUI.h"

void Editor::init()
{
	UI::create_tab("Material", material_tab, Material::none);
	UI::create_tab("Texture", texture_tab, Texture::none);
}
