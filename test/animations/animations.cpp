#include "animations.h"
#include "../PlayerScript.h"
#include "../CameraScript.h"

class Printer : public Script
{
	void draw(Transform *t)
	{
		Debug::drawPoint(t->getPosition());
		for (Transform *c : t->getChildren())
		{
			Debug::drawLine(t->getPosition(), c->getPosition(), vec3(0.0f, 1.0f, 0.0f));
			draw(c);
		}

		if (Input::getKeyDown(sf::Keyboard::A))
			find<Animator>()->play(0);
	}
	void update()
	{
		draw(tr);
	}
};

void test_animations()
{
	Input::setCursorMode(CursorMode::Capture);
	PhysicEngine::get()->setGravity(vec3(0.0f));



	//Entity *root = Scene::import("Knight/knight.dae");
	Entity *root = Scene::import("Bob/bob_lamp_update.md5mesh");
	root->insert<Printer>();


	MaterialRef m = Material::getDefault();
	m->set("albedoMap", Texture::get("Iron/albedo.png"));
	m->set("metallicMap", Texture::get("Iron/metallic.png"));
	m->set("roughnessMap", Texture::get("Iron/roughness.png"));

	MeshRef groundMesh = Mesh::createQuad(MeshData::Basic, vec2(20.0f), uvec2(2), uvec2(2));
	Entity::create("Ground", false)
		->insert<Graphic>(groundMesh);

	MeshRef mesh = Mesh::createSphere(MeshData::Basic, 0.25f);
	Entity *object = Entity::create("Object", true)
		->insert<Graphic>(mesh);

	/*
	for (vec3 pos : {vec3(1, 0, 0), vec3(-1, 0, 0), vec3(1, 0, 1), vec3(-1, 0, 1)})
		object = Entity::clone(object, pos+vec3(0,0,0.5f), vec3(0.0f, 0.0f, Random::next<float>()));
	*/

	// Camera
	Entity::create("MainCamera", false, vec3(0.0f, 2.0f, 2.0f))
		->insert<Camera>(70, 0.1f, 1000.0f, vec3(0.67f, 0.92f, 1.0f))
		->insert<Skybox>()
		->insert<CameraScript>(nullptr, 0.2f, 5.0f);
}
