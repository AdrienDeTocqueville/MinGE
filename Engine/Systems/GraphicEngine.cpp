#include "Systems/GraphicEngine.h"
#include "Renderer/UBO.h"
#include "Utility/Debug.h"
#include "Assets/Program.h"

#include "Components/Animator.h"
#include "Components/Graphic.h"
#include "Components/Camera.h"
#include "Components/Light.h"

#include "Entity.h"

#include <thread>
#include <cstring>
#include <unordered_set>

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

void GraphicEngine::editBuffer(GLenum _target, unsigned _size, const void* _data)
{
	void* adress = glMapBuffer(_target, GL_WRITE_ONLY);

	if (adress == nullptr)
	{
		Error::add(OPENGL_ERROR, "GraphicEngine::editBuffer() -> glMapBuffer() returns nullptr");
		return;
	}

	memcpy(adress, _data, _size);

	glUnmapBuffer(_target);
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
	cameras.push_back(_camera);
	sortBuckets();
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
	auto it = std::find(cameras.begin(), cameras.end(), _camera);
	if (it != cameras.end())
	{
		*it = cameras.back();
		cameras.pop_back();
	}

	sortBuckets();
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

void GraphicEngine::sortBuckets()
{
	std::unordered_set<RenderTarget*> targets;

	buckets.clear();
	for (Camera *camera : cameras)
	{
		RenderTarget *target = camera->getRenderTarget().get();
		if (targets.find(target) == targets.end())
			buckets.push_back(&(target->bucket));
	}

	std::sort(buckets.begin(), buckets.end(), [] (CommandBucket *&a, CommandBucket *&b) {
		return (a->target->getPriority() > b->target->getPriority());
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
	//glEnable(GL_SCISSOR_TEST);

	// TODO : find a solution for that
	{
		Light *source = GraphicEngine::get()->getLight();
		Program::setBuiltin("lightPosition", source->getPosition());
		Program::setBuiltin("lightColor", source->getColor());
	}

	for (Camera* camera: cameras)
		camera->update();

	for (Animator* animator: animators)
		animator->animate();

//#define ENQUEUE_THREAD
#ifdef ENQUEUE_THREAD
	auto queue_commands = [=](int i, int last)
	{
		while (i < last)
		{
			for (CommandBucket *bucket : buckets)
				graphics[i]->render(bucket);
			i++;
		}
	};

	int NUM_THREADS = 3;
	int curr = 0, step = graphics.size() / (NUM_THREADS + 1);
	std::thread threads[NUM_THREADS];
	for (int i = 0; i < NUM_THREADS; i++)
	{
		threads[i] = std::thread(queue_commands, curr, curr + step);
		curr += step;
	}
	queue_commands(curr, graphics.size());

	for (auto& th : threads) th.join();
#else
	for (Graphic* graphic: graphics)
		for (CommandBucket *bucket : buckets)
			graphic->render(bucket);
#endif

	// Sort
	for (CommandBucket *bucket : buckets)
		bucket->sort();

	// Submit to backend
	for (CommandBucket *bucket : buckets)
	{
		bucket->submit();
		bucket->clear();
	}

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
