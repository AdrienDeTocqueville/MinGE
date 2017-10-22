#include "Systems/GraphicEngine.h"

#include "Components/Graphic.h"
#include "Components/Camera.h"
#include "Components/Light.h"

#include "Physic/AABB.h"
#include "Entity.h"

#include <cstring>

GraphicEngine* GraphicEngine::instance = nullptr;
unsigned GraphicEngine::renderTarget = GE_DEPTH_COLOR;

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

//    glEnable(GL_BLEND);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CLIP_DISTANCE0);

    glPointSize(7);
    glLineWidth(3);
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

    if(adress == nullptr)
    {
        Error::add(OPENGL_ERROR, "GraphicEngine::editBuffer() -> glMapBuffer() returns nullptr");
        return;
    }

//    std::copy(data, data+size, adress);
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
    if (_camera != nullptr)
        cameras.push_back(_camera);

    if (Camera::main == nullptr && _camera->getEntity()->getTag() == "MainCamera")
        Camera::main = _camera;
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

    // Replace if it was the main camera
    if (_camera == Camera::main)
    {
        Camera::main = nullptr;

        auto _cameras = Entity::findAllByTag("MainCamera");

        for (Entity* _c: _cameras)
            if (_c->get<Camera>() != _camera)
            {
                Camera::main = _c->get<Camera>();
                return;
            }
    }
}

void GraphicEngine::removeLight(Light* _light)
{
    lights.remove(_light);
}

void GraphicEngine::render()
{
    glEnable(GL_SCISSOR_TEST);

    for (Camera* camera: cameras)
    {
        if (camera == Camera::main)
            continue;

        camera->use();

        for (Graphic* graphic: graphics)
        {
            setMatrix(GE_MODEL);
            graphic->render();
        }
    }

    Camera::main->use();

    for (Graphic* graphic: graphics)
    {
        setMatrix(GE_MODEL);
        graphic->render();
    }

    AABB::draw();

    Debug::update();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDisable(GL_SCISSOR_TEST);
}

/// Setters
void GraphicEngine::setMatrix(const MatrixType _type, mat4 _value)
{
    matrices[_type] = _value;
}

void GraphicEngine::updateCameraViewPort() const
{
    for (Camera* camera: cameras)
    {
        if (camera == Camera::main)
        camera->computeViewPort();
//        camera->resizeFramebuffer();
    }
}

/// Getters
mat4 GraphicEngine::getMatrix(const MatrixType _type) const
{
    return matrices[_type];
}

Light* GraphicEngine::getLight() const
{
    if (!lights.empty())
        return lights.front();

    return nullptr;
}

//vec3 GraphicEngine::getViewPosition() const
//{
//    mat3 rotMat(matrices[GE_VIEW]);
//    vec3 d(matrices[GE_VIEW][3]);
//
//    vec3 pos = -d * rotMat;
//
//    return pos;
//}
