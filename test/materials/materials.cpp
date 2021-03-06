#include "materials.h"
#include "../PlayerScript.h"
#include "../CameraScript.h"

void test_materials()
{
	Random::setSeed(42);

	MeshRef mesh = Mesh::createCube();

	MaterialRef standard = Material::getDefault();
	Entity *object = Entity::create("Object", true)
		->insert<Graphic>(nullptr);

	const std::vector<std::string> mat_names = {"Iron", "Greasy", "Grimy"};
	std::vector<MaterialRef> mats(mat_names.size());
	for (size_t i(0); i < mat_names.size(); i++)
	{
		mats[i] = standard->clone();
		mats[i]->define({ "COLOR_MAP", "METALLIC_MAP", "ROUGHNESS_MAP", "MAIN_LIGHT" });

		mats[i]->set("color_map", Texture::get(mat_names[i] + "/albedo.png"));
		mats[i]->set("metallic_map", Texture::get(mat_names[i] + "/metallic.png"));
		mats[i]->set("roughness_map", Texture::get(mat_names[i] + "/roughness.png"));
	}

	for (int i(0); i < 41; i++)
	for (int j(0); j < 21; j++)
	{
		auto e = Entity::clone(object, vec3(0.0f, i-20.0f, j-10.0f), vec3(0.0f), vec3(0.5f));
		e->find<Graphic>()->setMesh(mesh, {mats[Random::next<int>(0, mat_names.size())]});
	}

	// Camera
	Entity::create("MainCamera", false, vec3(-15.0f, 0.0f, 0.0f))
		->insert<Camera>(70, 0.1f, 1000.0f, vec3(0.67f, 0.92f, 1.0f))
		->insert<Skybox>()
		->insert<CameraScript>(nullptr, 0.2f, 7.0f, vec3(-0.23171, 0.91854, 0.32032));
}
