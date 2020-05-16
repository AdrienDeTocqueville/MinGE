#include "Editor.h"

void material_tab();

void Editor::init()
{
	UI::create_tab("Material", material_tab);
}
