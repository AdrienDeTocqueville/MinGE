#pragma once

#include "Renderer/RenderContext.h"
#include "Assets/Material.h"

#include "Utility/helpers.h"


#define BUFFER_OFFSET(offset) ((char*)nullptr + (offset))

class Animator;
class Graphic;
class Camera;
class Light;

struct View
{
	mat4 vp;
	ivec4 viewport;
	vec4 clear_color;
	vec3 view_pos;

	unsigned clear_flags;
	unsigned fbo;
	RenderPass::Type pass;
};

class GraphicEngine
{
	friend class Engine;

public:
	/// Methods (static)
	static GraphicEngine* get();

	/// Methods (public)
	void addAnimator(Animator* _animator);
	void addGraphic(Graphic* _graphic);
	void addCamera(Camera* _camera);
	void addLight(Light* _light);

	void removeAnimator(Animator* _animator);
	void removeGraphic(Graphic* _graphic);
	void removeCamera(Camera* _camera);
	void removeLight(Light* _light);

	void sortCameras();
	void onResize(class RenderTarget *target);
	void toggleWireframe();
	void render();

	/// Getters
	Light* getLight() const;

	//vec3 getViewPosition() const;

	static const size_t MAX_VIEWS = 8;

private:
	/// Methods (private)
	GraphicEngine();
	~GraphicEngine();

	void clear();

	static void create();
	static void destroy();

	/// Attributes (private)
	std::vector<Animator*> animators;
	std::vector<Graphic*> graphics;
	std::vector<Camera*> cameras;
	std::vector<Light*> lights;

	View views[MAX_VIEWS];
	RenderContext *contexts;

	bool wireframe = false;

	/// Attributes (static)
	static GraphicEngine* instance;
};
