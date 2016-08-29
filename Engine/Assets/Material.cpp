#include "Assets/Material.h"

#include "Components/Light.h"
#include "Components/Camera.h"
#include "Components/Transform.h"

#include "Systems/GraphicEngine.h"

std::map<std::string, Material*> Material::materials;
Material* Material::base = nullptr;

Material::Material(std::string _name):
    name(_name)
{
    materials[name] = this;
}

/// Methods (static)
Material* Material::get(std::string _name)
{
    auto it = materials.find(_name);

    if (it == materials.end())
    {
        Error::add(MINGE_ERROR, "Material::get() -> unknown material: " + _name);
        return nullptr;
    }
    else
        return it->second;
}

void Material::clear()
{
    for(auto it: materials)
        delete it.second;

    materials.clear();

    Material::base = nullptr;
}

/// ModelMaterial
bool ModelMaterial::use(Transform* _tr)
{
    unsigned _target = GraphicEngine::renderTarget;

    _tr->use();

    programs[_target]->use();

    if (_target == GE_DEPTH_COLOR)
    {
        programs[_target]->send(0, GraphicEngine::get()->getMatrix(GE_MVP));
        programs[_target]->send(1, GraphicEngine::get()->getMatrix(GE_MODEL));
        programs[_target]->send(2, mat3(transpose(inverse(GraphicEngine::get()->getMatrix(GE_MODEL)))));

        programs[_target]->send(3, Camera::current->getClipPlane());
        programs[_target]->send(4, Camera::current->getTransform()->position);

        Light* light = GraphicEngine::get()->getLight();
        if (light)
        {
            vec3 attenuation(light->getAttenuation());

            programs[_target]->send(5, light->getPosition());
            programs[_target]->send(6, light->getDiffuseColor());
            programs[_target]->send(7, light->getAmbientCoefficient());

            programs[_target]->send(8, attenuation.x);
            programs[_target]->send(9, attenuation.y);
            programs[_target]->send(10, attenuation.z);
        }

        programs[_target]->send(11, 0);  // Texture

        programs[_target]->send(12, ambient);
        programs[_target]->send(13, diffuse);
        programs[_target]->send(14, specular);
        programs[_target]->send(15, exponent);

        texture->use();
    }
    else if (_target == GE_DEPTH)
        programs[_target]->send(0, GraphicEngine::get()->getMatrix(GE_MVP));

    return true;
}

bool AnimatedModelMaterial::use(Transform* _tr)
{
    unsigned _target = GraphicEngine::renderTarget;

    _tr->use();

    programs[_target]->use();

    if (_target == GE_DEPTH_COLOR)
    {
        programs[_target]->send(0, GraphicEngine::get()->getMatrix(GE_MVP));
        programs[_target]->send(1, GraphicEngine::get()->getMatrix(GE_MODEL));
        programs[_target]->send(2, transpose(inverse( GraphicEngine::get()->getMatrix(GE_MODEL) )));

        programs[_target]->send(3, Camera::current->getClipPlane());


        programs[_target]->send(4, &matrices);


        programs[_target]->send(5, Camera::current->getTransform()->position);

        Light* light = GraphicEngine::get()->getLight();
        if (light)
        {
            vec3 attenuation(light->getAttenuation());

            programs[_target]->send(6, light->getPosition());
            programs[_target]->send(7, light->getDiffuseColor());
            programs[_target]->send(8, light->getAmbientCoefficient());

            programs[_target]->send(9, attenuation.x);
            programs[_target]->send(10, attenuation.y);
            programs[_target]->send(11, attenuation.z);
        }

        programs[_target]->send(12, 0);  // Texture

        programs[_target]->send(13, ambient);
        programs[_target]->send(14, diffuse);
        programs[_target]->send(15, specular);
        programs[_target]->send(16, exponent);

        texture->use();
    }
    else if (_target == GE_DEPTH)
        programs[_target]->send(0, GraphicEngine::get()->getMatrix(GE_MVP));

    return true;
}

/// TerrainMaterial
bool TerrainMaterial::use(Transform* _tr)
{
    unsigned _target = GraphicEngine::renderTarget;

    _tr->use();

    programs[_target]->use();

    if (_target == GE_DEPTH_COLOR)
    {
        programs[_target]->send(0, GraphicEngine::get()->getMatrix(GE_MVP));
        programs[_target]->send(1, GraphicEngine::get()->getMatrix(GE_MODEL));
        programs[_target]->send(2, mat3(transpose(inverse(GraphicEngine::get()->getMatrix(GE_MODEL)))));

        programs[_target]->send(3, Camera::current->getClipPlane());

        programs[_target]->send(4, side);
        programs[_target]->send(5, detailScale);

        for (int i(TEX_COUNT-1) ; i >= 0 ; i--) // /!\ int because i is going to be negative
        {
            programs[_target]->send(i+6, i+2);

            textures[i]->use(i+2);
        }
        glActiveTexture(GL_TEXTURE0);
    }
    else if (_target == GE_DEPTH)
        programs[_target]->send(0, GraphicEngine::get()->getMatrix(GE_MVP));

    return true;
}

/// SkyboxMaterial
bool SkyboxMaterial::use(Transform* _tr)
{
    if (GraphicEngine::renderTarget == GE_DEPTH)
        return false;

    _tr->use();

    programs[0]->use();

        programs[0]->send(0, GraphicEngine::get()->getMatrix(GE_MVP));

    return true;
}

/// GUIMaterial
bool GUIMaterial::use(Transform* _tr)
{
    if (GraphicEngine::renderTarget == GE_DEPTH || texture == nullptr || Camera::current != Camera::main)
        return false;

    _tr->use();

    programs[0]->use();

        programs[0]->send(0, GraphicEngine::get()->getMatrix(GE_MODEL));
        programs[0]->send(1, 0);
        texture->use();

    return true;
}
