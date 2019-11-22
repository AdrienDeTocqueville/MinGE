#pragma once

#include "Renderer/GLDriver.h"
#include "Renderer/CommandBucket.h"

#include "Utility/helpers.h"


#define BUFFER_OFFSET(offset) ((char*)nullptr + (offset))

class Entity;

class Animator;
class Graphic;
class Camera;
class Light;

class GraphicEngine
{
	friend class Engine;

	public:
		/// Methods (static)
			static GraphicEngine* get();

			static void editBuffer(GLenum _target, unsigned _size, const void* _data);

		/// Methods (public)
			void addAnimator(Animator* _animator);
			void addGraphic(Graphic* _graphic);
			void addCamera(Camera* _camera);
			void addLight(Light* _light);

			void removeAnimator(Animator* _animator);
			void removeGraphic(Graphic* _graphic);
			void removeCamera(Camera* _camera);
			void removeLight(Light* _light);

			void sortBuckets();
			void onResize(RenderTarget *target);
			void toggleWireframe();
			void render();

		/// Getters
			Light* getLight() const;

			//vec3 getViewPosition() const;

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

			std::vector<CommandBucket*> buckets;

			bool wireframe = false;

		/// Attributes (static)
			static GraphicEngine* instance;
};
