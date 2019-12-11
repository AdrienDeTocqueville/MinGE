#include "animations.h"
#include "../PlayerScript.h"
#include "../CameraScript.h"

class Printer : public Script
{
	void start()
	{
		if (auto g = find<Graphic>())
		{
			auto aabb = g->getAABB();
			float scale = 1.0f / aabb.dim().z;

			//vec3 pos = -aabb.center() * scale;
			//pos.z += 0.5f;
			//tr->setPosition(pos);

			scale = 0.0025f;
			tr->setScale(vec3(scale));
		}
		speed = vec2(0.0f);
	}
	void draw(Transform *t)
	{
		Debug::drawPoint(t->getPosition());
		for (Transform *c : t->getChildren())
		{
			Debug::drawLine(t->getPosition(), c->getPosition(), vec3(0.0f, 1.0f, 0.0f));
			draw(c);
		}
	}
	void update()
	{
		if (draw_skel)
			draw(tr);

		if (Input::getKeyPressed(sf::Keyboard::L))
			draw_skel = !draw_skel;

		float delta = 0.5f * Time::deltaTime;
		float max_s = 1.0f;

		if (Input::getKeyDown(sf::Keyboard::Z))
			speed.y = min(max_s, speed.y + delta);
		else if (Input::getKeyDown(sf::Keyboard::S))
			speed.y = max(0.0f, speed.y - delta);

		if (Input::getKeyDown(sf::Keyboard::Q))
			speed.x = max(-max_s, speed.x - delta);
		else if (Input::getKeyDown(sf::Keyboard::D))
			speed.x = min(max_s, speed.x + delta);

		if (length(speed) > 1.0f)
		speed = normalize(speed);
		Debug::drawVector(vec3(0.0f), vec3(-speed.x, -speed.y, 0.0f), vec3(193,23,182)/255.0f);
		find<Animator>()->setMotion(speed);
	}

	vec2 speed;
	bool draw_skel = false;
};

void test_animations()
{
	//Entity *root = Scene::import("Knight/knight.dae");
	//Entity *root = Scene::import("Bob/bob_lamp_update.md5mesh");


	/*
	Entity *root = Scene::import("Model/model.fbx");
	if (root)
	{
		const Skeleton &skeleton = root->find<Animator>()->getSkeleton();
		root->find<Animator>()->setMotionBlender({
			//{vec2( 0, 1.0f), Scene::import_animation("Model/run.fbx", skeleton)},
			{vec2( 0, 1.0f), Scene::import_animation("Model/walk.fbx", skeleton)},
			{vec2( 0, 0.0f), Scene::import_animation("Model/idle.fbx", skeleton)},

			//{vec2(-1.0f, 0), Scene::import_animation("Model/left_run.fbx", skeleton)},
			{vec2(-1.0f, 0), Scene::import_animation("Model/left.fbx", skeleton)},

			//{vec2( 1.0f, 0), Scene::import_animation("Model/right_run.fbx", skeleton)},
			{vec2( 1.0f, 0), Scene::import_animation("Model/right.fbx", skeleton)},
		});
		root->insert<Printer>();
	}
	*/


	MaterialRef m = Material::getDefault();
	m->set("albedoMap", Texture::get("Iron/albedo.png"));
	m->set("metallicMap", Texture::get("Iron/metallic.png"));
	m->set("roughnessMap", Texture::get("Iron/roughness.png"));

	MeshRef groundMesh = Mesh::createQuad(MeshData::Basic, vec2(20.0f), uvec2(2), uvec2(2));
	Entity::create("Ground", false)
		->insert<Graphic>(groundMesh);

	MeshRef mesh = Mesh::createSphere(MeshData::Basic, 0.25f);
	Entity::create("Object", true)
		->insert<Graphic>(mesh);

	/*
	for (vec3 pos : {vec3(1, 0, 0), vec3(-1, 0, 0), vec3(1, 0, 1), vec3(-1, 0, 1)})
		object = Entity::clone(object, pos+vec3(0,0,0.5f), vec3(0.0f, 0.0f, Random::next<float>()));
	*/

	// Camera
	Entity::create("MainCamera", false, vec3(0.0f, 2.0f, 2.0f))
		->insert<Camera>(70, 0.1f, 1000.0f, vec3(0.67f, 0.92f, 1.0f))
		->insert<Skybox>()
		->insert<CameraScript>(nullptr, 0.2f, 0.0f);
		//->insert<CameraScript>(root->find<Transform>(), 0.2f, 5.0f, vec3(0,0,1/0.0025f));
		//->insert<CameraScript>(root->find<Transform>(), 0.2f, 5.0f);
}
