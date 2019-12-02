#include "Systems/GraphicEngine.h"

#include "Renderer/RenderContext.inl"
#include "Renderer/CommandKey.h"
#include "Renderer/Commands.h"
#include "Renderer/UBO.h"

#include "Utility/Debug.h"
#include "Assets/Program.h"
#include "Profiler/profiler.h"

#include "Components/Animator.h"
#include "Components/Graphic.h"
#include "Components/Camera.h"
#include "Components/Skybox.h"
#include "Components/Light.h"

#include "Entity.h"

#include <thread>
#include <cstring>
#include <algorithm>

GraphicEngine* GraphicEngine::instance = nullptr;

/// Methods (private)
GraphicEngine::GraphicEngine()
{
	GLenum code = glewInit();

	if(code != GLEW_OK)
	{
		std::string errorString(reinterpret_cast<const char*>(glewGetErrorString(code)));
		Error::add(OPENGL_ERROR, "glewInit() -> Failed with error: " + errorString);
	}

	std::cout << std::endl;
	printf("Opengl version: (%s)\n", glGetString(GL_VERSION));
	printf("GLSL   version: (%s)\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_SCISSOR_TEST);

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glEnable(GL_CLIP_DISTANCE0);

	glPointSize(7);
	glLineWidth(3);

	UBO::setupPool();
	Program::init();
}

GraphicEngine::~GraphicEngine()
{ }

void GraphicEngine::clear()
{
	animators.clear();
	graphics.clear();
	cameras.clear();
	lights.clear();
}

/// Methods (static)
void GraphicEngine::create()
{
	if (instance != nullptr)
		return;

	instance = new GraphicEngine();
}

void GraphicEngine::destroy()
{
	delete instance;
	instance = nullptr;
}

GraphicEngine* GraphicEngine::get()
{
	return instance;
}

/// Methods (public)
void GraphicEngine::addAnimator(Animator* _animator)
{
	animators.push_back(_animator);
}

void GraphicEngine::addGraphic(Graphic* _graphic)
{
	graphics.push_back(_graphic);
}

void GraphicEngine::addCamera(Camera* _camera)
{
	if (cameras.size() == MAX_VIEWS)
		return Error::add(USER_ERROR, "GraphicEngine::addCamera() -> You reached max number of cameras");

	cameras.push_back(_camera);
	sortCameras();
}

void GraphicEngine::addLight(Light* _light)
{
	lights.push_back(_light);
}

void GraphicEngine::removeAnimator(Animator* _animator)
{
	auto it = std::find(animators.begin(), animators.end(), _animator);
	if (it != animators.end())
	{
		*it = animators.back();
		animators.pop_back();
	}
}

void GraphicEngine::removeGraphic(Graphic* _graphic)
{
	auto it = std::find(graphics.begin(), graphics.end(), _graphic);
	if (it != graphics.end())
	{
		*it = graphics.back();
		graphics.pop_back();
	}
}

void GraphicEngine::removeCamera(Camera* _camera)
{
	// keep cameras sorted
	auto it = std::find(cameras.begin(), cameras.end(), _camera);
	if (it != cameras.end())
		cameras.erase(it);
}

void GraphicEngine::removeLight(Light* _light)
{
	auto it = std::find(lights.begin(), lights.end(), _light);
	if (it != lights.end())
	{
		*it = lights.back();
		lights.pop_back();
	}
}

void GraphicEngine::sortCameras()
{
	std::sort(cameras.begin(), cameras.end(), [] (Camera *&a, Camera *&b) {
		RenderTarget *ta = a->getRenderTarget().get(), *tb = b->getRenderTarget().get();
		return (ta->getPriority() > tb->getPriority());
	});
}

void GraphicEngine::onResize(RenderTarget *target)
{
	for (Camera *camera : cameras)
		if (camera->getRenderTarget().get() == target)
			camera->computeViewPort();
}

void GraphicEngine::toggleWireframe()
{
	wireframe = !wireframe;

	glPolygonMode(GL_FRONT_AND_BACK, wireframe? GL_LINE: GL_FILL);
}

void GraphicEngine::render()
{
	MICROPROFILE_SCOPEI("SYSTEM_GRAPHIC", "render");

	// Animate skeletal meshes
	{ MICROPROFILE_SCOPEI("SYSTEM_GRAPHIC", "animate");
	for (Animator* animator: animators)
		animator->animate();
	}


	//glEnable(GL_SCISSOR_TEST);

	// TODO : find a solution for that
	{
		Light *source = GraphicEngine::get()->getLight();
		Program::setBuiltin("lightPosition", source->getPosition());
		Program::setBuiltin("lightColor", source->getColor());
	}

	{ MICROPROFILE_SCOPEI("SYSTEM_GRAPHIC", "cameras");
	for (size_t i(0); i < cameras.size(); i++)
	{
		cameras[i]->update(views + i);

		auto *cmd = contexts[0].create<SetupView>(); cmd->view = views + i;
		contexts[0].add(CommandKey::encode(i, views[i].pass, 0), cmd);

		if (Skybox* sky = cameras[i]->find<Skybox>())
		{
			contexts[0].add(CommandKey::encode(i, RenderPass::Skybox), contexts[0].create<SetupSkybox>());
			sky->render(contexts + 0, i);
		}
	}
	}

	// Create commands
	{ MICROPROFILE_SCOPEI("SYSTEM_GRAPHIC", "create commands");
	for (Graphic* graphic: graphics)
		graphic->render(contexts + 0, cameras.size(), views);
	}

	// Merge
	RenderContext::CommandPair *pairs;
	size_t cmd_count = 0;

	{ MICROPROFILE_SCOPEI("SYSTEM_GRAPHIC", "merge contexts");
	for (int i(0); i < NUM_THREADS; i++)
		cmd_count += contexts[i].cmd_count();

	pairs = new RenderContext::CommandPair[cmd_count];
	auto *dest = (uint8_t*)pairs;
	for (int i(0); i < NUM_THREADS; i++)
	{
		const auto &pool = contexts[i].commands;
		for (int b(0); b < pool.block; b++)
		{
			memcpy(dest, pool.blocks[b], pool.block_bytes);
			dest += pool.block_bytes;
		}
		memcpy(dest, pool.blocks[pool.block], pool.index);
		dest += pool.index;
	}
	}
	
	// Sort
	{ MICROPROFILE_SCOPEI("SYSTEM_GRAPHIC", "sort commands");
	std::sort(pairs, pairs + cmd_count);
	}

	// Submit to backend
	{ MICROPROFILE_SCOPEI("SYSTEM_GRAPHIC", "submit commands");
	auto *count = pairs + cmd_count;
	for (auto *pair = pairs; pair < count; pair++)
		CommandPacket::submit(pair->key, pair->packet);
	}

	delete pairs;
	for (int i(0); i < NUM_THREADS; i++)
		contexts[i].clear();

#ifdef DRAWAABB
	for(Graphic* g: graphics)
		g->getAABB().prepare();

	AABB::draw();
#endif

#ifdef DEBUG
	Debug::update();
#endif

	//glDisable(GL_SCISSOR_TEST);
}

/// Getters
Light* GraphicEngine::getLight() const
{
	if (!lights.empty())
		return lights.back();

	return nullptr;
}

/*
vec3 GraphicEngine::getViewPosition() const
{
	mat3 rotMat(matrices[GE_VIEW]);
	vec3 d(matrices[GE_VIEW][3]);

	vec3 pos = -d * rotMat;

	return pos;
}
*/
