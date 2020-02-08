#include "sky.h"
#include "../PlayerScript.h"
#include "../CameraScript.h"

void test_sky()
{
	MaterialRef m = Material::getDefault();
	m->define({ "COLOR_MAP", "METALLIC_MAP", "ROUGHNESS_MAP" });

	std::string name = Random::element({"Iron", "Greasy", "Grimy"});
	m->set("color_map", Texture::get(name + "/albedo.png"));
	m->set("metallic_map", Texture::get(name + "/metallic.png"));
	m->set("roughness_map", Texture::get(name + "/roughness.png"));


	MeshRef groundMesh = Mesh::createQuad(MeshData::Basic, vec2(20.0f), uvec2(2), uvec2(2));
	Entity *ground = Entity::create("Ground", false);
		ground->insert<Graphic>(groundMesh);

	// Camera
	Entity *cam = Entity::create("MainCamera", false, vec3(0.0f, 0.0f, 2.0f));
		cam->insert<Camera>(70, 0.1f, 1000.0f, vec3(0.67f, 0.92f, 1.0f));
		cam->insert<Skybox>();
		cam->insert<CameraScript>(nullptr, 0.2f, 1.0f);
}
