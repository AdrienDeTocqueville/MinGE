#include "Systems/GraphicEngine.h"
#include "Renderer/UBO.h"
#include "Utility/Debug.h"
#include "Assets/Program.h"

#include "Components/Graphic.h"
#include "Components/Camera.h"
#include "Components/Light.h"

#include "Entity.h"

#include <cstring>

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
	updateCamerasOrder();
}

void GraphicEngine::addLight(Light* _light)
{
	if (_light != nullptr)
		lights.push_back(_light);
}

void GraphicEngine::removeGraphic(Graphic* _graphic)
{
	graphics.remove(_graphic);
}

void GraphicEngine::removeCamera(Camera* _camera)
{
	cameras.remove(_camera);
	updateCamerasOrder();
}

void GraphicEngine::removeLight(Light* _light)
{
	lights.remove(_light);
}

void GraphicEngine::updateCamerasOrder()
{
	cameras.sort([](Camera *a, Camera *b) {
		return a->getRenderingOrder() > b->getRenderingOrder();
	});

	buckets.clear();
	for (Camera* camera: cameras)
	{
		for (CommandBucket &bucket : camera->buckets)
			buckets.push_back(&bucket);
	}
}

void GraphicEngine::toggleWireframe()
{
	wireframe = !wireframe;

	glPolygonMode(GL_FRONT_AND_BACK, wireframe? GL_LINE: GL_FILL);
}

void GraphicEngine::render()
{
	//glEnable(GL_SCISSOR_TEST);

	/*
	2 buckets:
	  - shadow
	 	key ( view | depth [front-back] | material )
	  - display
	 	key ( view | layer | translucency | material | depth [front-back] )
	  - post-processing?
	  - hud ?
	
	view:
		viewport, scissor, camera settings...
	layer:
		world, skybox, hud
	transflucency:
		normal, additive

	OR
	remove the view from the key since its always there and always first (note: how about CSM ?)
	embed it in the command bucket
	create one bucket for each view
	-> more buckets = better multithreading i guess
	 + sorting might be faster with smaller buckets
	*/
	/*
	typedef ShadowKey uint16_t;
	typedef DisplayKey uint64_t;

	template <typename Key>
	struct CommandBucket
	{
	};

	CommandBucket<ShadowKey> shadow_bucket;
	CommandBucket<DisplayKey> display_bucket;
	*/

	// foreach shadow_light
	//	submit_to(shadow_bucket, shadow_light.state)

	// foreach camera
	//	submit_to(display_bucket, camera.state)

	// foreach graphic
	//	if cast_shadows
	//		foreach shadow_light
	//			submit_to(shadow_bucket, graphic.drawcall_depth)
	//	foreach camera
	//		submit_to(display_bucket, graphic.drawcall_color_depth)
	//
	//
	// sort(shadow_bucket)
	// sort(display_bucket)
	//
	// <threads join>
	//
	// submit(shadow_bucket)
	// submit(display_bucket)

	/*
	for (Camera* camera: cameras)
		camera->update();

	for (Graphic* graphic: graphics)
		for (CommandBucket *bucket : buckets)
			graphic->add(bucket);
	*/

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
	{
		camera->use();

		for (Graphic* graphic: graphics)
			graphic->render();
	}

#ifdef DRAWAABB
	for(Graphic* g: graphics)
		g->getAABB().prepare();

	AABB::draw();
#endif

#ifdef DEBUG
	Debug::update();
#endif

	GL::BindVertexArray(0);
	//glDisable(GL_SCISSOR_TEST);
}

/// Setters
void GraphicEngine::updateCameraViewPort() const
{
	for (Camera* c : cameras)
		c->computeViewPort();
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
