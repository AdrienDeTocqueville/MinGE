#ifndef CAMERA_H
#define CAMERA_H

#include "Components/Component.h"

#include "Assets/RenderTexture.h"

class Entity;
class Transform;

class Camera : public Component
{
    friend class GraphicEngine;

    public:
        Camera(float _fovy, float _zNear, float _zFar, vec3 _color = vec3(0.0f), RenderTexture* _renderTexture = nullptr, bool _perspective = true,
               vec4 _viewport = vec4(0.0f,0.0f,1.0f,1.0f), unsigned _flags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        virtual ~Camera();

        /// Methods (public)
            virtual Camera* clone() const override;

            virtual void registerComponent() override;
            virtual void deregisterComponent() override;

            void use();

        /// Setters
            void setRenderTexture(RenderTexture* _renderTexture);
            void setClipPlane(vec4 _clipPlane);

        /// Getters
            Texture* getColorBuffer() const;
            RenderBuffer* getDepthBuffer() const;

            vec4 getClipPlane() const;

            vec3 getPosition() const;
            vec3 getDirection() const;

        /// Attributes (static)
            static Camera* main;
            static Camera* current;

    private:
        /// Methods (private)
            void computeViewPort();

            void createFramebuffer();

        /// Attributes
            float fovy, zNear, zFar, FPS;

            vec3 clearColor;
            unsigned clearFlags;

            bool perspective;

            vec4 viewport;
            vec4 relViewport;    // in screen coordinates (between 0 and 1)

            vec4 clipPlane;

            mat4 view, projection;

            // Framebuffer
            unsigned fbo;

            RenderTexture* renderTexture;
};

vec3 getSymetric(vec3 _point, vec4 _plane);

#endif // CAMERA_H
