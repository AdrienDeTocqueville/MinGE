#pragma once

#include "Renderer/GLDriver.h"
#include "Renderer/CommandBucket.h"

#include "Utility/helpers.h"


#define BUFFER_OFFSET(offset) ((char*)nullptr + (offset))

class Entity;
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
			void addGraphic(Graphic* _graphic);
			void addCamera(Camera* _camera);
			void addLight(Light* _light);

			void removeGraphic(Graphic* _graphic);
			void removeCamera(Camera* _camera);
			void removeLight(Light* _light);

			void sortCameras();
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
			std::list<Graphic*> graphics;
			std::list<Camera*> cameras;
			std::list<Light*> lights;

			bool wireframe = false;

		/// Attributes (static)
			static GraphicEngine* instance;
};
