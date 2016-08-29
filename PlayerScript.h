#ifndef PLAYERSCRIPT_H
#define PLAYERSCRIPT_H

#include <MinGE.h>

class BillardScript : public Script
{
    public:
        BillardScript();
        virtual ~BillardScript()
        { }

        /// Methods (public)
            void start() override
            {
                rb = getComponent<RigidBody>();
            }

            void update() override;

    private:
        RigidBody* rb;

        vec3 point;
        float angleX, angleY;
        sf::Clock timer;
        bool load;
};

class GolfScript : public Script
{
    public:
        GolfScript();
        virtual ~GolfScript()
        { }

        /// Methods (public)
            void start() override
            {
                rb = getComponent<RigidBody>();
            }

            void update() override;

    private:
        RigidBody* rb;

        float intensite;
        int niveau;
};

class PlayerScript : public Script
{
    public:
        PlayerScript(Entity* _prot, float _speed, float _minSpeed, float _maxSpeed);

        virtual ~PlayerScript()
        {
//            for (unsigned i(0) ; i < 4 ; i++)
//                co[i]->destroy();
        }

        /// Methods (public)
            void start() override
            {
                rb = getComponent<RigidBody>();

                Entity* cube = Entity::findByTag("Cube2");
                if (cube != nullptr)
                {
                    dj = new DistanceConstraint(rb, vec3(0, 0, 0.5f),
                                           cube->getComponent<RigidBody>(), vec3(0, 0, -0.5f), 3);
//                    PhysicEngine::get()->addConstraint(dj);
                }

                shapes[0] = Mesh::createCube();
                shapes[1] = Mesh::createCylinder();
                shapes[2] = Mesh::createCylinder(Material::base, ALLFLAGS, 0.5f, 0.0f);
                shapes[3] = Mesh::createSphere();

                co[0] = new Box();
                co[1] = new Cylinder();
                co[2] = new Cone();
                co[3] = new Sphere();

                keys[0] = sf::Keyboard::Num1;
                keys[1] = sf::Keyboard::Num2;
                keys[2] = sf::Keyboard::Num3;
                keys[3] = sf::Keyboard::Num4;

                entity->addComponent(new Graphic(shapes[0]));

                for (unsigned i(0) ; i < 4 ; i++)
                    co[i]->attach(entity);

                current = 0;
                entity->addComponent(co[current]);
            }

            void update() override;

    private:
        Entity* prot;
        RigidBody* rb;

        float speed;
        float minSpeed, maxSpeed;
        float fireRate = 0.75f, time = 0.0f;

        DistanceConstraint* dj;

        Mesh* shapes[4];
        Collider* co[4];
        sf::Keyboard::Key keys[4];
        int current;
};

class RayScript: public Script
{
    void start() override
    {
    }

    void lateUpdate() override
    {
    }

    Camera* c;
    Transform* t;
};

class RayMat: public Material
{
    public:
        RayMat(std::string _name):
            Material(_name)
        {
            programs.push_back(Program::get("ray.vert", "ray.frag"));
        }

        ~RayMat()
        { }

        bool use(Transform* _tr) override
        {
            _tr->use();

            programs[0]->use();

            programs[0]->send(0, Camera::main->getTransform()->position);
            programs[0]->send(1, Camera::main->getDirection());
            programs[0]->send(2, factor);

            return true;
        }
        float factor = 3;
};

class sc: public Script
{
    public:
    sc(GUIMaterial* _g): g(_g) {}

    void start() override
    {
        g->texture = Entity::findByTag("Refract")->getComponent<Camera>()->getColorBuffer();
    }

    GUIMaterial* g;
};

class AnimScript: public Script
{
    public:
    AnimScript() {}
    virtual ~AnimScript() {}

    void start() override
    {
        model = static_cast<AnimatedModel*>(getComponent<Graphic>()->getMesh());

        anim = -1;
        aim = 0;
        fire = 0;
    }

    void update() override
    {
        int save = anim;

        bool sprint = Input::getKeyDown(sf::Keyboard::LShift);
//        if (Input::getKeyPressed(sf::Keyboard::A)) aim = 1-aim;
//        if (Input::getKeyPressed(sf::Keyboard::E)) fire = 1-fire;
        if (Input::getMousePressed(sf::Mouse::Right)) aim = 1-aim;
        fire = Input::getMouseDown(sf::Mouse::Left);

        if (Input::getKeyDown(sf::Keyboard::Space))
        {
            anim = 13;
        }
        else if (Input::getKeyDown(sf::Keyboard::Z))
        {
            tr->position += vec3(0, -1, 0)*Component::deltaTime*(sprint?2.0f:1.0f);
            anim = 3 + sprint*3 + aim + 2*fire - (aim&&fire);
        }
        else if (Input::getKeyDown(sf::Keyboard::S))
        {
            tr->position += vec3(0, 1, 0)*Component::deltaTime*(sprint?2.0f:1.0f);
            anim = 9+sprint*1;
        }
        else if (Input::getKeyDown(sf::Keyboard::D))
        {
            tr->position += vec3(-1, 0, 0)*Component::deltaTime;
            anim = 11;
        }
        else if (Input::getKeyDown(sf::Keyboard::Q))
        {
            tr->position += vec3(1, 0, 0)*Component::deltaTime;
            anim = 12;
        }
        else
            anim = 0 + aim + 2*fire - (aim&&fire);

        tr->toMatrix();

        if (anim != save)
            model->loadAnimation(anim, true);

        model->updateSkeleton();
    }

    AnimatedModel* model;
    int anim;
    int aim, fire;
};

#endif // PLAYERSCRIPT_H
