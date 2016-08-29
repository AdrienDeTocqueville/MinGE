#ifndef WATERSCRIPT_H
#define WATERSCRIPT_H

#include <MinGE.h>

class WaterMat: public Material
{
    public:
        WaterMat(std::string _name):
            Material(_name)
        {
            programs.push_back(Program::get("water.vert", "water.frag"));
        }

        ~WaterMat()
        { }

        bool use(Transform* _tr) override
        {
            if (Camera::current != Camera::main)
                return false;

            _tr->use();

            programs[0]->use();

            programs[0]->send(0, GraphicEngine::get()->getMatrix(GE_MVP));

            programs[0]->send(1, tiling);
            programs[0]->send(2, distorsionFactor);
            programs[0]->send(3, offset);
//
            for (unsigned i(0) ; i < texCount ; i++)
            {
                programs[0]->send(i+4, i);
                texture[i]->use(i);
            }

            return true;

        }

        float tiling = 3.0f;
        float distorsionFactor = 0.015f;
        float offset = 0.0f;

        static const unsigned texCount = 3;
        Texture* texture[texCount] =
        {
            Texture::get("Textures/0.png"),
            Texture::get("Textures/0.png"),
            Texture::get("Textures/DistorsionMap.png")
//            Texture::get("Textures/NormalMap.png")
        };
};

class WaterScript : public Script
{
    public:
        WaterScript(WaterMat* _w)
        {
            w = _w;
        }

        virtual ~WaterScript()
        {

        }

        void start() override
        {
            reflect = Entity::findByTag("Reflect")->getComponent<Camera>();
            refract = Entity::findByTag("Refract")->getComponent<Camera>();

            reflect->setClipPlane(vec4(0, 0, 1, -tr->position.z)); // Clip au dessous de z=-9.5f
            refract->setClipPlane(vec4(0, 0, -1, tr->position.z)); // Clip au dessus de z=-9.5f

            w->texture[0] = reflect->getColorBuffer();
            w->texture[1] = refract->getColorBuffer();
        }

        void lateUpdate() override
        {
            w->offset += deltaTime*0.15f;

            Transform* t;

            t = reflect->getTransform();
                t->position = Camera::main->getTransform()->position;
                vec3 target = Camera::main->getTransform()->position + Camera::main->getDirection();

                t->position.z -= 2.0f * (t->position.z - tr->position.z);
                     target.z -= 2.0f * (     target.z - tr->position.z);

                t->lookAt(target);


            t = refract->getTransform();
                t->position = Camera::main->getTransform()->position;
                t->rotation = Camera::main->getTransform()->rotation;
        }

        WaterMat* w;
        Camera* reflect;
        Camera* refract;
};

#endif // WATERSCRIPT_H
