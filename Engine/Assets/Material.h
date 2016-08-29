#ifndef MATERIAL_H
#define MATERIAL_H

#include "Assets/Program.h"
#include "Assets/Texture.h"

class Transform;
class Light;

class Material
{
    public:
        Material(std::string _name);
        virtual ~Material() {}

        /// Methods (public)
            virtual bool use(Transform* _tr) = 0;

        /// Methods (static)
            static Material* get(std::string _name);
            static void clear();

        /// Attributes (static)
            static std::map<std::string, Material*> materials;

            static Material* base;

    protected:
        std::string name;

        std::vector<Program*> programs;
};

/// Default materials
class ModelMaterial : public Material
{
    public:
        ModelMaterial(std::string _name, std::string _texture = "Textures/0.png"):
            Material(_name),
            ambient(0.3f, 0.3f, 0.3f),
            diffuse(0.8f, 0.8f, 0.8f),
            specular(0.0f,0.0f, 0.0f),
            exponent(1.0f)
        {
            programs.push_back(Program::get("object.vert", "object.frag"));

            texture = Texture::get(_texture);
        }

        ~ModelMaterial()
        { }

        bool use(Transform* _tr) override;

        vec3 ambient;
        vec3 diffuse;
        vec3 specular;
        float exponent;

        Texture* texture = nullptr;
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

        ~AnimatedModelMaterial()
        { }

        bool use(Transform* _tr) override;

        vec3 ambient;
        vec3 diffuse;
        vec3 specular;
        float exponent;

        Texture* texture = nullptr;

        std::vector<mat4> matrices;
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

        ~TerrainMaterial()
        { }

        bool use(Transform* _tr) override;


        static const unsigned TEX_COUNT = 6;
        Texture* textures[TEX_COUNT] = {nullptr};

        unsigned side = 0;
        vec4 detailScale = vec4(0.0f);

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

        ~SkyboxMaterial()
        { }

        bool use(Transform* _tr) override;
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

        ~GUIMaterial()
        { }

        bool use(Transform* _tr) override;

        Texture* texture;
};

#endif // MATERIAL_H
