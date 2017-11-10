#include "Components/Camera.h"
#include "Components/Skybox.h"
#include "Components/Transform.h"

#include "Utility/Input.h"

Camera* Camera::main = nullptr;
Camera* Camera::current = nullptr;

Camera::Camera(float _FOV, float _zNear, float _zFar, vec3 _color, RenderTexture* _renderTexture, bool _orthographic, vec4 _viewport, unsigned _flags):
    FOV(_FOV), zNear(_zNear), zFar(_zFar),
    clearColor(_color), clearFlags(_flags),
    orthographic(_orthographic),
    relViewport(_viewport),
    clipPlane(0, 0, 0, 1000),
    fbo(0), renderTexture(_renderTexture)
{
    computeViewPort();

    createFramebuffer();
}

Camera::~Camera()
{
    glDeleteFramebuffers(1, &fbo);

    delete renderTexture;
}

/// Methods (public)
Camera* Camera::clone() const
{
    return new Camera(FOV, zNear, zFar, clearColor, renderTexture, orthographic, relViewport, clearFlags);
}

void Camera::use()
{
    current = this;

    glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
    glScissor (viewport.x, viewport.y, viewport.z, viewport.w);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);


    view = glm::lookAt(tr->getToWorldSpace(vec3(0.0f)),
                       tr->getToWorldSpace(vec3(1, 0, 0)),
                       vec3(0, 0, 1));

    GraphicEngine::get()->setMatrix(GE_VP, projection * view);


    // Background
    glClearColor(clearColor.r, clearColor.g, clearColor.b, 0.0f);

    glClear(clearFlags);

    Skybox* sky = find<Skybox>();

    if (sky)
        sky->render();

}

/// Setters
void Camera::setRenderTexture(RenderTexture* _renderTexture)
{
    delete renderTexture;

    renderTexture = _renderTexture;

    glDeleteFramebuffers(1, &fbo);
    fbo = 0;
    createFramebuffer();
}

void Camera::setClipPlane(vec4 _clipPlane)
{
    clipPlane = _clipPlane;
}

/// Getters
Texture* Camera::getColorBuffer() const
{
    if (renderTexture)
        return renderTexture->getColorBuffer();
    else
        return nullptr;
}

RenderBuffer* Camera::getDepthBuffer() const
{
    if (renderTexture)
        return renderTexture->getDepthBuffer();
    else
        return nullptr;
}

vec4 Camera::getClipPlane() const
{
    return clipPlane;
}

vec3 Camera::getPosition() const
{
    return tr->getToWorldSpace(vec3(0, 0, 0));
}

vec3 Camera::getDirection() const
{
    return tr->getVectorToWorldSpace(vec3(1, 0, 0));
}

float Camera::getFOV() const
{
    return FOV;
}

float Camera::getAspectRatio() const
{
    return viewport.z/viewport.w;
}

/// Methods (private)
void Camera::onRegister()
{
    GraphicEngine::get()->addCamera(this);
}

void Camera::onDeregister()
{
    GraphicEngine::get()->removeCamera(this);
}

void Camera::computeViewPort()
{
    vec2 ws;
    if (renderTexture)
        ws = renderTexture->getSize();
    else
        ws = Input::getWindowSize();

    viewport = vec4((int)(relViewport.x * ws.x),
                    (int)(relViewport.y * ws.y),
                    (int)(relViewport.z * ws.x),
                    (int)(relViewport.w * ws.y));

    float aspectRatio = viewport.z/viewport.w;

    if (orthographic)
        projection = ortho(-FOV*0.5f, FOV*0.5f, -FOV*0.5f/aspectRatio, FOV*0.5f/aspectRatio, zNear, zFar);
    else
        projection = perspective(FOV, aspectRatio, zNear, zFar);

}

void Camera::createFramebuffer()
{
    if (renderTexture == nullptr)
        return;

    glGenFramebuffersEXT(1, &fbo);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    /// Color attachment
        if (renderTexture->getColorBuffer() != nullptr)
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture->getColorBuffer()->getId(), 0);

    /// Depth attachement
        if (renderTexture->getDepthBuffer() != nullptr)
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderTexture->getDepthBuffer()->getId());


	int val = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);

    if (val != GL_FRAMEBUFFER_COMPLETE)
		Error::add(OPENGL_ERROR, "Camera::createFramebuffer() -> glCheckFramebufferStatus() returns: " + val);

    if (fbo != 0)
    {
        GLuint buf[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, buf);
    }
}

/// Other
vec3 getSymetric(vec3 _point, vec4 _plane)
{
    float k = (_plane.x*_point.x + _plane.y*_point.y + _plane.z*_point.z + _plane.w) / (_plane.x + _plane.y + _plane.z);
    vec3 h(_point - k * vec3(_plane));
    return 2.0f * h - _point;
}
