#ifndef MATERIAL_H
#define MATERIAL_H

#include "Assets/Program.h"
#include "Assets/Texture.h"

class Transform;
class Light;

class Material
{
    friend class Mesh;
    friend class Engine;
    friend class Graphic;

    public:
        Material(std::string _name);

        /// Methods (public)
            virtual Material* clone() const = 0;
            virtual bool use(Transform* _tr) = 0;

        /// Methods (static)
            static void clear();

        /// Attributes (static)
            static std::vector<Material*> materials;
            static Material* base;

    protected:
        virtual ~Material() {}

        std::string name;

        std::vector<Program*> programs;
};

/// Default materials
class ModelMaterial : public Material
{
    public:
        ModelMaterial(std::string _name, std::string _texture = "Textures/white.png"):
            Material(_name),
            ambient(0.3f, 0.3f, 0.3f),
            diffuse(0.8f, 0.8f, 0.8f),
            specular(0.0f,0.0f, 0.0f),
            exponent(1.0f)
        {
            programs.push_back(Program::get("object.vert", "object.frag"));

            texture = Texture::get(_texture);
        }

        bool use(Transform* _tr) override;

        vec3 ambient;
        vec3 diffuse;
        vec3 specular;
        float exponent;

        Texture* texture = nullptr;

        Material* clone() const
        { return new ModelMaterial(*this); }

    protected:
        virtual ~ModelMaterial() {}
};

class AnimatedModelMaterial : public Material
{
    public:
        AnimatedModelMaterial(std::string _name, std::string _texture = "Textures/0.png"):
            Material(_name),
            ambient(0.3f, 0.3f, 0.3f),
            diffuse(0.8f, 0.8f, 0.8f),
            specular(0.0f,0.0f, 0.0f),
            exponent(8.0f)
        {
            programs.push_back(Program::get("skeletal.vert", "object.frag"));

            texture = Texture::get(_texture);
        }

        bool use(Transform* _tr) override;

        vec3 ambient;
        vec3 diffuse;
        vec3 specular;
        float exponent;

        Texture* texture = nullptr;

        std::vector<mat4>* matrices;

        Material* clone() const
        { return new AnimatedModelMaterial(*this); }

    protected:
        virtual ~AnimatedModelMaterial() {}
};

class TerrainMaterial : public Material
{
    public:
        TerrainMaterial(std::string _name):
            Material(_name)
        {
            programs.push_back(Program::get("object.vert", "terrain.frag"));
//            programs.push_back(Program::get("light.vert" , ""));
        }

        bool use(Transform* _tr) override;


        static const unsigned TEX_COUNT = 6;
        Texture* textures[TEX_COUNT] = {nullptr};

        unsigned side = 0;
        vec4 detailScale = vec4(0.0f);

        Material* clone() const
        { return new TerrainMaterial(*this); }

    protected:
        virtual ~TerrainMaterial() {}
};

/// Skyobox
class SkyboxMaterial: public Material
{
    public:
        SkyboxMaterial(std::string _name):
            Material(_name)
        {
            programs.push_back(Program::get("skydome.vert", "skydome.frag"));
        }

        bool use(Transform* _tr) override;

        Material* clone() const
        { return new SkyboxMaterial(*this); }

    protected:
        virtual ~SkyboxMaterial() {}
};

/// GUI
class GUIMaterial: public Material
{
    public:
        GUIMaterial(std::string _name):
            Material(_name), texture(nullptr)
        {
            programs.push_back(Program::get("gui.vert", "gui.frag"));
        }

        bool use(Transform* _tr) override;

        Texture* texture;

        Material* clone() const
        { return new GUIMaterial(*this); }

    protected:
        virtual ~GUIMaterial() {}
};

#endif // MATERIAL_H
