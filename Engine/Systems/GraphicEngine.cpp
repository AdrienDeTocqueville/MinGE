#include "Systems/GraphicEngine.h"
#include "Renderer/UBO.h"
#include "Utility/Debug.h"
#include "Assets/Program.h"

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

//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_CLIP_DISTANCE0);

	glPointSize(7);
	glLineWidth(3);

	UBO::setupPool();
	Program::init();
}

GraphicEngine::~GraphicEngine()
{ }

void GraphicEngine::clear()
{
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
void GraphicEngine::addGraphic(Graphic* _graphic)
{
	if (_graphic != nullptr)
		graphics.push_back(_graphic);
}

void GraphicEngine::addCamera(Camera* _camera)
{
	cameras.push_back(_camera);
	sortBuckets();
}

void GraphicEngine::addLight(Light* _light)
{
	if (_light != nullptr)
		lights.push_back(_light);
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
	cameras.remove(_camera);
	sortBuckets();
}

void GraphicEngine::removeLight(Light* _light)
{
	lights.remove(_light);
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
		Program::setBuiltin("diffuseColor", source->getDiffuseColor());
		Program::setBuiltin("ambientCoefficient", source->getAmbientCoefficient());
		Program::setBuiltin("aConstant", source->getAttenuation().x);
		Program::setBuiltin("aLinear", source->getAttenuation().y);
		Program::setBuiltin("aQuadratic", source->getAttenuation().z);
	}

	for (Camera* camera: cameras)
		camera->update();

	auto queue_commands = [=](int i, int last)
	{
		while (i < last)
		{
			for (CommandBucket *bucket : buckets)
				graphics[i]->render(bucket);
			i++;
		}
	};


	/*
	int NUM_THREADS = 4;
	int curr = 0, step = graphics.size() / NUM_THREADS;
	std::thread threads[NUM_THREADS];
	for (int i = 0; i < NUM_THREADS-1; i++)
	{
		threads[i] = std::thread(queue_commands, curr, curr + step);
		curr += step;
	}
	threads[NUM_THREADS-1] = std::thread(queue_commands, curr, graphics.size());

	for (auto& th : threads) th.join();
	*/

	for (Graphic* graphic: graphics)
		for (CommandBucket *bucket : buckets)
			graphic->render(bucket);

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
		return lights.front();

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
