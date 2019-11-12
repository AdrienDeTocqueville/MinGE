#include "sky.h"
#include "../PlayerScript.h"
#include "../CameraScript.h"

void test_sky()
{
	Input::setCursorMode(CursorMode::Capture);
	PhysicEngine::get()->setGravity(vec3(0.0f));

	std::string name = Random::element({"Iron", "Greasy", "Grimy"});
	MaterialRef m = Material::getDefault();
	m->set("albedoMap", Texture::get(name + "/albedo.png"));
	m->set("metallicMap", Texture::get(name + "/metallic.png"));
	m->set("roughnessMap", Texture::get(name + "/roughness.png"));


	MeshRef groundMesh = Mesh::createQuad(MeshData::Basic, vec2(20.0f), uvec2(2), uvec2(2));
	Entity *object = Entity::create("Ground", false)
		->insert<Graphic>(groundMesh);

	// Camera
	Entity::create("MainCamera", false, vec3(0.0f, 0.0f, 2.0f))
		->insert<Camera>(70, 0.1f, 1000.0f, vec3(0.67f, 0.92f, 1.0f))
		->insert<Skybox>()
		->insert<CameraScript>(nullptr, 0.2f, 1.0f);
}
