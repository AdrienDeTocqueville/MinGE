#ifndef TEXTURE_H
#define TEXTURE_H

#include "includes.h"

class Texture
{
    public:
        Texture(unsigned _width, unsigned _height);

        virtual ~Texture();

        /// Method (public)
            void use(unsigned _active = 0) const;

        /// Methods (static)
            static Texture* get(std::string _path);
            static void clear();

        /// Getter
            unsigned getId() const { return texture; }

    private:
        /// Constructors
            Texture();
            Texture(std::string _path);

        /// Operator
            Texture operator=(Texture _original) = delete;

        /// Attributes (static)
            static std::map<std::string, Texture*> textures;

        /// Attributes (private)
            std::string path;
            unsigned texture;
};

class RenderBuffer
{
    public:
        RenderBuffer(unsigned _width, unsigned _height, GLenum _format);
        ~RenderBuffer();

        /// Getter
            unsigned getId() const { return renderBuffer; }

    private:
        /// Attributes (private)
            unsigned width, height;

            unsigned renderBuffer;
            GLenum format;
};

#endif // TEXTURE_H
