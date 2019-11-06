#pragma once

#include "Renderer/GLDriver.h"
#include "Renderer/CommandBucket.h"

#include "Utility/helpers.h"


#define BUFFER_OFFSET(offset) ((char*)nullptr + (offset))

enum MatrixType {GE_MODEL, GE_MVP, GE_VP};

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

			void updateCamerasOrder();
			void toggleWireframe();
			void computeMVP();
			void render();

		/// Setters
			void setMatrix(const MatrixType _type, mat4 _value = mat4(1.0f));

			void updateCameraViewPort() const;

			void setLock(bool _lock);

		/// Getters
			mat4 getMatrix(const MatrixType _type) const;
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

			std::vector<CommandBucket*> buckets;

			mat4 matrices[3];
			Light* light = nullptr;

			bool wireframe = false;

		/// Attributes (static)
			static GraphicEngine* instance;
};
