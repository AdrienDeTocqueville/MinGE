#include "Systems/GraphicEngine.h"

#include "Renderer/UBO.h"
#include "Assets/Shader.h"
#include "Utility/JobSystem/JobSystem.h"

#include "Components/Animator.h"
#include "Components/Graphic.h"
#include "Components/Camera.h"
#include "Components/Skybox.h"
#include "Components/Light.h"

GraphicEngine* GraphicEngine::instance = nullptr;

/// Methods (private)
GraphicEngine::GraphicEngine()
{
	GLenum code = glewInit();

	if(code != GLEW_OK)
	{
		std::string errorString(reinterpret_cast<const char*>(glewGetErrorString(code)));
		Error::add(Error::OPENGL, "glewInit() -> Failed with error: " + errorString);
	}

	std::cout << std::endl;
	printf("Opengl version: (%s)\n", glGetString(GL_VERSION));
	printf("GLSL   version: (%s)\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_SCISSOR_TEST);

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glEnable(GL_CLIP_DISTANCE0);

	glDepthFunc(GL_LEQUAL);
	glPointSize(7);
	glLineWidth(3);

	contexts = new RenderContext[JobSystem::worker_count()];

	UBO::setupPool();
	Shader::setupBuiltins();
}

GraphicEngine::~GraphicEngine()
{
	delete[] contexts;
}

void GraphicEngine::clear()
{
	Shader::clear();
	Texture::clear();

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
		return Error::add(Error::USER, "GraphicEngine::addCamera() -> You reached max number of cameras");

	cameras.push_back(_camera);
	sortCameras();
}

void GraphicEngine::addLight(Light* _light)
{
	if (Light::main == nullptr && _light->type == Light::Directional)
		Light::main = _light;

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

	if (Light::main == _light)
	{
		for (Light *l : lights)
		{
			if (l->type == Light::Directional)
			{
				Light::main = l;
				break;
			}
		}
	}
}

void GraphicEngine::sortCameras()
{
	std::sort(cameras.begin(), cameras.end(), [] (Camera *&a, Camera *&b) {
		return (a->getPriority() > b->getPriority());
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
