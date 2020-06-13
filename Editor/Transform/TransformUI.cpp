#include <UI/UI.h>
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
	{}
}

struct ActionData
{
	TransformSystem *sys;
	Entity e;
};

static void edit_entity(TransformSystem *sys, Entity e)
{
	if (!sys->has(e)) return;
	if (!ImGui::CollapsingHeader(comp_name, ImGuiTreeNodeFlags_DefaultOpen)) return;

	Transform tr = sys->get(e);
	ActionData data { sys, e };

	vec3 pos = tr.position();
	Editor::field("Position", ImGui::DragFloat3, pos, data,
	(void (*)(vec3*,vec3*,ActionData*))[](vec3 *old, vec3 *val, ActionData *d) {
		if (d->sys->has(d->e))
			d->sys->get(d->e).set_position(*val);
	});

	vec3 rot = glm::degrees(tr.euler_angles());
	Editor::field("Rotation", ImGui::DragFloat3, rot, data,
	(void (*)(vec3*,vec3*,ActionData*))[](vec3 *old, vec3 *val, ActionData *d) {
		if (d->sys->has(d->e))
			d->sys->get(d->e).set_rotation(glm::radians(*val));
	});

	vec3 scale = tr.scale();
	Editor::field("Scale", ImGui::DragFloat3, scale, data,
	(void (*)(vec3*,vec3*,ActionData*))[](vec3 *old, vec3 *val, ActionData *d) {
		if (d->sys->has(d->e))
			d->sys->get(d->e).set_scale(*val);
	});
}

const system_editor_t editor = []() {
	system_editor_t e{};
	e.type_name = "TransformSystem";

	e.add_system = NULL;
	e.add_component = decltype(e.add_component)(add_component);
	e.edit_entity = decltype(e.edit_entity)(edit_entity);

	/*
	t.destroy = [](void *system) { ((TransformSystem*)system)->~TransformSystem(); };
	t.update = NULL;
	t.on_destroy_entity = NULL;
	t.save = (void(*)(void*, SerializationContext&))serialize;
	t.load = [](void *system, const SerializationContext &ctx) { new(system) TransformSystem(ctx); };
	*/
	return e;
}();

}
