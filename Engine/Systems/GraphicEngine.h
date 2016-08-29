#ifndef RENDERER_H
#define RENDERER_H

#include "includes.h"
#include "Systems/Debug.h"

#define BUFFER_OFFSET(offset) ((char*)nullptr + (offset))

#define GE_DEPTH_COLOR 0
#define GE_DEPTH 1
#define GE_COLOR 2

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

	        void computeMVP()   { matrices[GE_MVP] = matrices[GE_VP]*matrices[GE_MODEL]; }

        	void render();

        /// Setters
	        void setMatrix(const MatrixType _type, mat4 _value = mat4(1.0f));

            void updateCameraViewPort() const;

            void setLock(bool _lock);

        /// Getters
	        mat4 getMatrix(const MatrixType _type) const;

	        Light* getLight() const;

//	        vec3 getViewPosition() const;

	        bool getLock() const;

        /// Attributes (static)
            static unsigned renderTarget;

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

            mat4 matrices[3];
            Light* light = nullptr;

        /// Attributes (static)
            static GraphicEngine* instance;
};

#endif // RENDERER_H
