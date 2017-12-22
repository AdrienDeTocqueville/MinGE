#ifndef RENDERBUFFER_H
#define RENDERBUFFER_H

#include "Assets/Texture.h"

class RenderTexture
{
    public:
        RenderTexture(unsigned _width, unsigned _height, unsigned _depth = 16);
        ~RenderTexture();

        Texture* getColorBuffer() const;
        RenderBuffer* getDepthBuffer() const;

        vec2 getSize() const;

    private:
        unsigned width, height;

        Texture* colorBuffer;
        RenderBuffer* depthBuffer;
};

#endif // RENDERBUFFER_H
