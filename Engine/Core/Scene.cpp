#include "Core/Scene.h"
#include "Core/AssetDatabase.h"

#include "IO/DataModel.h"
#include "Structures/ArrayList.h"

#include "Core/SceneImporter.inl"

array_list_t<Scene::scene_t> Scene::scenes;

Scene Scene::create(const char *name)
{
	uint32_t i = scenes.add();
	Scene::scene_t &scene = scenes[i];

	scene.create();
	if (name != nullptr)
		scene.name = strdup(name);

	return Scene::get(i);

}

static void destroy_scene(UID32 object)
{
	Scene scene(object);
	ASSERT(scene.is_valid(), "Trying to destroy an invalid Scene handle");

	Scene::scene_t &data = Scene::scenes[object.id()];

	data.destroy();
	Scene::scenes.remove(object.id());
}

void Scene::destroy()
{
	destroy_scene(*this);
}

static UID32 transfer(UID32 object, DataModel *model)
{
	if (!object.valid())
		object = Scene::create(NULL);
	Scene asset = Scene(object);
	
	Scene::scene_t &data = Scene::scenes[asset.id()];
	model->transfer("name", data.name);
	model->transfer("data", data.serialized_scene);
	
	return object;
}

const object_type_t Scene::type = []() {
	object_type_t t = INIT_ASSET_TYPE(Scene);

	t.destroy = destroy_scene;
	t.data = [](UID32 obj) { return (object_data_t*)&(Scene::scenes[obj.id()]); };
	t.transfer = transfer;
	return t;
}();