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
		setState(0);
		auto anim = find<Animator>()->getAnimations()[2];
		auto skel = find<Animator>()->getSkeleton();

		Animation::Track &t = anim->channels[skel.bone_index["shoulder.L"]];
		std::cout << t.loop << "  " << t.bone_index << "  " << t.keys.size() << std::endl;
		for (auto &key : t.keys)
		{
			std::cout << key.time << "\t";
			write(glm::eulerAngles(key.rot));
		}

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

		if (Input::getKeyDown(sf::Keyboard::Space))
			setState(2);
		else
		{
			if (Input::getKeyDown(sf::Keyboard::Z))
				setState(1);
			else if (Input::getKeyDown(sf::Keyboard::Q))
				setState(3);
			else if (Input::getKeyDown(sf::Keyboard::D))
				setState(4);
			else
				setState(0);
		}
	}

	void setState(int _state)
	{
		if (state != _state)
			find<Animator>()->play(_state);
		state = _state;
	}

	int state = -1;
	bool draw_skel = false;
};

void test_animations()
{
	Input::setCursorMode(CursorMode::Capture);
	PhysicEngine::get()->setGravity(vec3(0.0f));



	//Entity *root = Scene::import("Knight/knight.dae");
	//Entity *root = Scene::import("Bob/bob_lamp_update.md5mesh");
	Entity *root = Scene::import("Model/model.fbx");

	if (root)
	{
		const Skeleton &skeleton = root->find<Animator>()->getSkeleton();
		root->find<Animator>()->addAnimation(Scene::import_animation("Model/idle.fbx", skeleton));
		root->find<Animator>()->addAnimation(Scene::import_animation("Model/walking.fbx", skeleton));
		root->find<Animator>()->addAnimation(Scene::import_animation("Model/jump.fbx", skeleton));
		root->find<Animator>()->addAnimation(Scene::import_animation("Model/left.fbx", skeleton));
		root->find<Animator>()->addAnimation(Scene::import_animation("Model/right.fbx", skeleton));
		root->insert<Printer>();
	}


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
		->insert<CameraScript>(root->find<Transform>(), 0.2f, 5.0f, vec3(0,0,1/0.0025f));
}
