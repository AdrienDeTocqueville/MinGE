#include <UI/UI.h>
#include <Render/Debug.h>
#include <Transform/Transform.h>

#include "Editor/Editor.h"
#include "Editor/Transform/TransformUI.h"

namespace TransformSystemUI
{

static const char *comp_name = "Transform";

static void add_component(TransformSystem *sys, const char *name, Entity e)
{
	if (sys->has(e)) return;

	if (ImGui::MenuItem(name))
		sys->add(e);
}

struct ActionData
{
	TransformSystem *sys;
	Entity e;
};

#define SIMPLE_PROP(label, widget, owner, type, prop) do { \
void (*func)(type*,type*,ActionData*) = [](type *old, type *val, ActionData *d) { \
	if (d->sys->has(d->e)) d->sys->get(d->e).set_##prop(*val); \
}; \
Editor::field(label, widget, owner.prop(), data, func); \
} while (0)


#define COMPLEX_PROP(label, widget, type, prop, getter, setter) do { \
void (*func)(type*,type*,ActionData*) = [](type *old, type *val, ActionData *d) { \
	if (d->sys->has(d->e)) d->sys->get(d->e).set_##prop(setter(*val)); \
}; \
Editor::field(label, widget, getter, data, func); \
} while (0)


static void edit_entity(TransformSystem *sys, Entity e)
{
	if (!sys->has(e)) return;
	if (!ImGui::CollapsingHeader(comp_name, ImGuiTreeNodeFlags_DefaultOpen)) return;

	Transform tr = sys->get(e);
	ActionData data { sys, e };

	SIMPLE_PROP("Position", ImGui::DragFloat3, tr, vec3, position);
	COMPLEX_PROP("Rotation", ImGui::DragFloat3, vec3, rotation, glm::degrees(tr.euler_angles()), glm::radians);
	SIMPLE_PROP("Scale", ImGui::DragFloat3, tr, vec3, scale);

	vec3 p = tr.position();
	Debug::vector(p, tr.vec_to_world(vec3(1, 0, 0)), vec3(1, 0, 0));
	Debug::vector(p, tr.vec_to_world(vec3(0, 1, 0)), vec3(0, 1, 0));
	Debug::vector(p, tr.vec_to_world(vec3(0, 0, 1)), vec3(0, 0, 1));
}

const system_editor_t editor = []() {
	system_editor_t e{};
	e.type_name = "TransformSystem";

	e.add_system = NULL;
	e.add_component = decltype(e.add_component)(add_component);
	e.edit_entity = decltype(e.edit_entity)(edit_entity);

	return e;
}();

}
