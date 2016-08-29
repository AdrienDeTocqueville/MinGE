#include <MinGE.h>

#include "Billard.h"
#include "Water.h"
#include "PlayerScript.h"
#include "TerrainScript.h"
#include "TPSCameraScript.h"

#define FULL_SCREEN sf::VideoMode::getDesktopMode()

#ifdef DEBUG
    #define VIDEOMODE sf::VideoMode(2*FULL_SCREEN.width/3, 2*FULL_SCREEN.height/3)
    #define STYLE sf::Style::Default
#else
    #define VIDEOMODE FULL_SCREEN
    #define STYLE sf::Style::Fullscreen
#endif

void loadBillard();
void loadGolf();
void loadTest();
void ray();
void water();
void heightfield();
void game();

int main()
{
    std::cout << "  -- MinGE --" << std::endl;
    srand (time(NULL));


    /// Create window
        sf::RenderWindow window(VIDEOMODE, "MinGE", STYLE, sf::ContextSettings(24, 0, 0, 4, 3));

    std::cout << window.getSize().x << "  " << window.getSize().y << std::endl;


    /// Create engine
        Engine* engine = new Engine(&window, 60);

    std::vector<std::function<void()>> scenes = { game, loadBillard, loadGolf, loadTest, ray, water, heightfield };
    int scene = 0;

    scenes[scene]();


    /// Main loop
        engine->start();
        Input::setCursorMode(GE_CAPTURE);

        bool lines = false;

        while ( !Input::isClosed() )
        {
            /// Handle events
                if (Input::getKeyReleased(sf::Keyboard::L))
                    AABB::drawAABBs = !AABB::drawAABBs;

                if (Input::getKeyReleased(sf::Keyboard::G))
                    PhysicEngine::get()->setGravity();

                if (Input::getKeyReleased(sf::Keyboard::Escape) || !Input::hasFocus())
                {
                    engine->setPause(true);
                    Input::setCursorMode(GE_FREE);
                }
                if (Input::getMousePressed(sf::Mouse::Left) && Input::hasFocus())
                {
                    engine->setPause(false);
                    Input::setCursorMode(GE_CAPTURE);
                }

                if (Input::getKeyReleased(sf::Keyboard::W))
                {
                    lines = !lines;
                    if (lines)
                        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    else
                        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                }

                if (Input::getKeyReleased(sf::Keyboard::Right))
                {
                    scene++;

                    if (scene >= (int)scenes.size())
                        scene = 0;

                    engine->clear();

                    scenes[scene]();
                    engine->start();
                }

                if (Input::getKeyReleased(sf::Keyboard::Left))
                {
                    if (scene == 0)
                        scene = scenes.size();

                    scene--;

                    engine->clear();

                    scenes[scene]();
                    engine->start();
                }

            /// Render
                if (engine->update())
                window.display();
        }

        std::cout << '\n' << '\n' << "Ending main loop" << std::endl;

    /// Delete resources
        delete engine;
        std::cout << "Engine deleted" << std::endl;

    #ifdef DEBUG
        sf::sleep(sf::seconds(1.0f));
    #endif // DEBUG

    return 0;
}

void loadBillard()
{
    PhysicEngine::get()->setGravity(vec3(0.0f));

    Model* boule = new Model("Models/Boule/Boule.obj");
    Model* table = new Model("Models/Table/Table.obj");

    PhysicMaterial* tapisMat = new PhysicMaterial("tapis");

    // Prototypes
        Entity* sphere = new Entity("Ball",
        {
            new Graphic(boule),
            new Sphere(),
            new RigidBody(10.0f)
        }, true);

    // Other entities
        new Entity("Table",
        {
            new Graphic(table),

            new Box(0.5f*vec3(0.0883976, 0.309844, 0.0235304), vec3(-0.510682, 0, 0.1189), tapisMat),
            new Box(0.5f*vec3(0.0883976, 0.309844, 0.0214589), vec3(0.505652, 0.00016192, 0.120502), tapisMat),

            new Box(0.5f*vec3(0.341392, 0.100701, 0.0222177), vec3( 0.234676, 0.249704, 0.119476), tapisMat),
            new Box(0.5f*vec3(0.341392, 0.100701, 0.0222177), vec3(-0.234676,  0.248362, 0.119476), tapisMat),

            new Box(0.5f*vec3(0.341392, 0.100701, 0.0222177), vec3( 0.234676,-0.249704, 0.119476), tapisMat),
            new Box(0.5f*vec3(0.341392, 0.100701, 0.0222177), vec3(-0.234676, -0.248362, 0.119476), tapisMat),

            new Box(0.5f*vec3(0.0412286, 0.345251, 0.106785), vec3( 0.442613, 0.00181496, 0.0544485), tapisMat),
            new Box(0.5f*vec3(0.851795, 0.398482, 0.106785), vec3(-0.00368398, 0.000893225, 0.0544485), tapisMat),
            new Box(0.5f*vec3(0.0371385, 0.345251, 0.106785), vec3(-0.447739, 0.00181496, 0.0544485), tapisMat),

            new Cylinder(0.04f, 0.151268, vec3(-0.440099, -0.225076, -0.0742403), tapisMat),
            new Cylinder(0.04f, 0.151268, vec3( 0.440099, -0.225076, -0.0742403), tapisMat),

            new Cylinder(0.04f, 0.151268, vec3( 0.440099,  0.225076, -0.0742403), tapisMat),
            new Cylinder(0.04f, 0.151268, vec3(-0.440099,  0.225076, -0.0742403), tapisMat),

            new RigidBody(0.0f),
            new Transform(vec3(0, 0, -6), vec3(0), 3.5f*vec3(6, 6, 6)),

            new Billard(sphere)
        });


        Entity* player = new Entity("Player",
        {
            new Sphere(),
            new Graphic(Mesh::createSphere()),
            new RigidBody(10.0f),
            new Transform(vec3(0, 0, 0), vec3(0), vec3(1.0f)),

            new BillardScript()
        });

        Entity* plane = new Entity("Walls",
        {
            new Graphic(Mesh::createQuad()),
            new Box(vec3(0.55f, 0.55f, 0.005f), vec3(0, 0, -0.005f)),
            new RigidBody(0.0f)
        }, true);


        plane->clone(vec3(0  , 0  , -10), vec3(0    , 0    , 0), vec3(50.0f));

        plane->clone(vec3(0  , -25, -5 ), vec3(-PI/2, 0    , 0), vec3(50, 10, 1));
        plane->clone(vec3(0  , 25 , -5 ), vec3(PI/2 , 0    , 0), vec3(50, 10, 1));
        plane->clone(vec3(25 , 0  , -5 ), vec3(0    , -PI/2, 0), vec3(10, 50, 1));
        plane->clone(vec3(-25, 0  , -5 ), vec3(0    , PI/2 , 0), vec3(10, 50, 1));


    // Light source
        ModelMaterial* bright = new ModelMaterial("bright");
            bright->ambient = vec3(10.0f/4.0f);
            bright->diffuse = vec3(0.0f);
            bright->specular = vec3(0.0f);
            bright->texture = Texture::get("Textures/white.png");

        new Entity("Light",
        {
            new Graphic(Mesh::createSphere(bright, ALLFLAGS, 0.25f)),
            new Light(GE_POINT_LIGHT, vec3(0.0f), vec3(0.9f), 0.7f, 1, 0.01, 0),
            new Transform(vec3(5, 2, 4))
        });

    // Camera
        new Entity("MainCamera",
        {
            new Camera(70, 0.1f, 1000.0f, vec3(0.67f, 0.92f, 1.0f)),
            new Transform(),
            new Skybox(),

            new TPSCameraScript(player->getTransform(), 0.2f, 7.0f, vec3(0, 0, 0.75f))
        });
}

void loadGolf()
{
    PhysicEngine::get()->setGravity(vec3(0.0f));

    Mesh* cubeMesh = Mesh::createCube();
    Mesh* sphereMesh = Mesh::createSphere();

    //Prototype
    Entity* cube = new Entity("Cube",
    {
        new Graphic(cubeMesh),
        new Box(),
        new RigidBody(0.0f)
    }, true);

        cube->clone(vec3(-1.0f, 7.0f, -0.5f),vec3(0.0f),vec3(3.0f, 24.0f, 1.0f));
        cube->clone(vec3(1.5f, 7.5f, -0.5f),vec3(0.0f),vec3(2.0f, 25.0f, 1.0f));
        cube->clone(vec3(-1.5f, 20.5f, -0.5f),vec3(0.0f),vec3(2.0f, 3.0f, 1.0f));
        cube->clone(vec3(1.0f, 21.0f, -0.5f),vec3(0.0f),vec3(3.0f, 2.0f, 1.0f));
        cube->clone(vec3(-2.5f, 8.5f, 0.0f),vec3(0.0f),vec3(0.5f, 27.0f, 0.75f));
        cube->clone(vec3(2.5f, 8.5f, 0.0f),vec3(0.0f),vec3(0.5f, 27.0f, 0.75f));
        cube->clone(vec3(0.0f, -5.0f, 0.0f),vec3(0.0f),vec3(5.0f, 0.5f, 0.75f));
        cube->clone(vec3(0.0f, 22.0f, 0.0f),vec3(0.0f),vec3(5.0f, 0.5f, 0.75f));


        cube->clone(vec3(-1.0f, 7.0f, 9.5f),vec3(0.0f),vec3(3.0f, 24.0f, 1.0f));
        cube->clone(vec3(1.5f, 7.5f, 9.5f),vec3(0.0f),vec3(2.0f, 25.0f, 1.0f));
        cube->clone(vec3(-1.5f, 20.5f, 9.5f),vec3(0.0f),vec3(2.0f, 3.0f, 1.0f));
        cube->clone(vec3(1.0f, 21.0f, 9.5f),vec3(0.0f),vec3(3.0f, 2.0f, 1.0f));
        cube->clone(vec3(-2.5f, 8.5f, 10.0f),vec3(0.0f),vec3(0.5f, 27.0f, 0.75f));
        cube->clone(vec3(2.5f, 8.5f, 10.0f),vec3(0.0f),vec3(0.5f, 27.0f, 0.75f));
        cube->clone(vec3(0.0f, -5.0f, 10.0f),vec3(0.0f),vec3(5.0f, 0.5f, 0.75f));
        cube->clone(vec3(0.0f, 22.0f, 10.0f),vec3(0.0f),vec3(5.0f, 0.5f, 0.75f));
        cube->clone(vec3(-1.0f, 5.0f, 10.0f),vec3(0.0f),vec3(2.5f, 0.5f, 0.75f));
        cube->clone(vec3(1.0f, 12.0f, 10.0f),vec3(0.0f),vec3(2.5f, 0.5f, 0.75f));

        cube->clone(vec3(-1.0f, 7.0f, 19.5f),vec3(0.0f),vec3(3.0f, 24.0f, 1.0f));
        cube->clone(vec3(1.5f, 7.5f, 19.5f),vec3(0.0f),vec3(2.0f, 25.0f, 1.0f));
        cube->clone(vec3(-1.5f, 20.5f, 19.5f),vec3(0.0f),vec3(2.0f, 3.0f, 1.0f));
        cube->clone(vec3(1.0f, 21.0f, 19.5f),vec3(0.0f),vec3(3.0f, 2.0f, 1.0f));
        cube->clone(vec3(-2.5f, 8.5f, 20.0f),vec3(0.0f),vec3(0.5f, 27.0f, 0.75f));
        cube->clone(vec3(2.5f, 8.5f, 20.0f),vec3(0.0f),vec3(0.5f, 27.0f, 0.75f));
        cube->clone(vec3(0.0f, -5.0f, 20.0f),vec3(0.0f),vec3(5.0f, 0.5f, 0.75f));
        cube->clone(vec3(0.0f, 22.0f, 20.0f),vec3(0.0f),vec3(5.0f, 0.5f, 0.75f));
        cube->clone(vec3(0.0f, 5.0f, 20.0f),vec3(PI/16,0.0f,0.0f),vec3(5.0f, 3.0f, 0.5f));
        cube->clone(vec3(0.0f, 12.0f, 20.5f),vec3(0.0f),vec3(5.0f, 1.0f, 1.5f));



    Entity* player = new Entity("Player",
    {
        new Graphic(sphereMesh),
        new Sphere(),

        new RigidBody(70.0f),
        new Transform(vec3(0.01, 0, 1), vec3(0), 0.5f*vec3(1, 1, 1)),

        new Light(GE_POINT_LIGHT, vec3(0, 0, 3), vec3(0.9f), 1.0f, 1, 0.01, 0),

        new GolfScript()
    });


    // Camera
    new Entity("MainCamera",
    {
        new Camera(70, 0.1f, 1000.0f, vec3(0.67f, 0.92f, 1.0f)),
        new Transform(),
        new Skybox(),

        new TPSCameraScript(player->getTransform(), 0.2f, 7.0f, vec3(0, 0, 0.75f))
    });
}

void loadTest()
{
    reinterpret_cast<ModelMaterial*>(Material::base)->texture = Texture::get("Textures/0.png");
    PhysicEngine::get()->setGravity(vec3(0.0f));

        Mesh* cubeMesh = Mesh::createCube();

        Model* boule = new Model("Models/Boule/Boule.obj");

    // Prototypes
        Entity* sphere = new Entity("Ball",
        {
            new Graphic(boule),
            new Sphere(0.5f),
            new RigidBody(10.0f)
        }, true);

        Entity* cube = new Entity("Cube",
        {
            new Graphic(cubeMesh),
            new Box(),
            new RigidBody(20.0f)
        }, true);

        Entity* cube2 = new Entity("Cube2",
        {
            new Graphic(cubeMesh),
            new Box(),
            new RigidBody(20.0f)
        }, true);

        Entity* plane = new Entity("Walls",
        {
            new Graphic(Mesh::createQuad()),
            new Box(vec3(0.55f, 0.55f, 0.005f), vec3(0, 0, -0.005f)),
            new RigidBody(0.0f)
        }, true);

    // Other entities
        Entity* player = new Entity("Player",
        {
            new RigidBody(20.0f),
            new Transform(vec3(0, 0, 0), vec3(0), vec3(1, 1, 1)),

            new PlayerScript(sphere, 10.0f, 2.0f, 30.0f)
        });

        Entity* ground = plane->clone(vec3(0  , 0  , -10), vec3(0, 0, 0), vec3(30.0f));

//        plane->clone(vec3(0  , -15, 5 ), vec3(-PI/2, 0    , 0), vec3(30.0f));
//        plane->clone(vec3(0  , 15 , 5 ), vec3(PI/2 , 0    , 0), vec3(30.0f));
//        plane->clone(vec3(15 , 0  , 5 ), vec3(0    , -PI/2, 0), vec3(30.0f));
//        plane->clone(vec3(-15, 0  , 5 ), vec3(0    , PI/2 , 0), vec3(30.0f));

//        new Entity("Plane",
//        {
//            new Graphic(cubeMesh),
//            new Box(),
//            new RigidBody(0.0f),
//            new Transform(vec3(0, 10.5, 1), vec3(0, 0, 0), vec3(30, 10, 1))
//        });
//
//        for( vec3 p: {vec3(-2, 0, 0), vec3(2, 0, 0), vec3(0, -2, 0), vec3(0, 2, 0)} )
//        {
//            Entity* c = cube2->clone(p);
//            DistanceConstraint* d = new DistanceConstraint(c->getComponent<RigidBody>(), vec3(0, 0, 0.5f),
//                                                 ground->getComponent<RigidBody>(), p*0.033f + vec3(0, 0, 0.33f), 3);
//
//            PhysicEngine::get()->addConstraint(d);
//        }
    // Light source
    ModelMaterial* bright = new ModelMaterial("bright");
        bright->ambient = vec3(10.0f/4.0f);
        bright->diffuse = vec3(0.0f);
        bright->specular = vec3(0.0f);
        bright->texture = Texture::get("Textures/white.png");

    new Entity("Light",
    {
        new Graphic(Mesh::createSphere(bright, ALLFLAGS, 0.25f)),
        new Light(GE_POINT_LIGHT, vec3(0.0f), vec3(0.9f), 1.0f, 1, 0.01, 0),
        new Transform(vec3(5, 2, 4))
    });


    // Camera
    new Entity("MainCamera",
    {
        new Camera(70, 0.1f, 1000.0f, vec3(0.67f, 0.92f, 1.0f)),
        new Transform(),
        new Skybox(),

        new TPSCameraScript(player->getTransform(), 0.2f, 7.0f, vec3(0, 0, 0.75f))
    });
}

void ray()
{
    reinterpret_cast<ModelMaterial*>(Material::base)->texture = Texture::get("Textures/0.png");
    PhysicEngine::get()->setGravity(vec3(0.0f));

        Mesh* cubeMesh = Mesh::createCube();
        RayMat* mat = new RayMat("ray");


    // Prototypes
        Entity* sphere = new Entity("Ball",
        {
//            new Graphic(Mesh::createSphere()),
            new Sphere(0.5f),
            new RigidBody(10.0f)
        }, true);

        Entity* plane = new Entity("Walls",
        {
            new Graphic(Mesh::createQuad()),
            new Box(vec3(0.55f, 0.55f, 0.005f), vec3(0, 0, -0.005f)),
            new RigidBody(0.0f)
        }, true);

    // Other entities
        Entity* player = new Entity("Player",
        {
            new Transform(vec3(0, 0, 0), vec3(0), vec3(1, 1, 1)),
            new RigidBody(10.0f),

            new PlayerScript(sphere, 10.0f, 2.0f, 30.0f)
        });

        new Entity("RayMarching",
        {
            new Graphic(Mesh::createQuad(mat, VERTICES|TEXCOORDS, vec2(1.0f))),

            new RayScript()
        });

//        Entity* ground = plane->clone(vec3(0  , 0  , -10), vec3(0    , 0    , 0), vec3(30.0f));
//
//        plane->clone(vec3(0  , -15, 5 ), vec3(-PI/2, 0    , 0), vec3(30.0f));
//        plane->clone(vec3(0  , 15 , 5 ), vec3(PI/2 , 0    , 0), vec3(30.0f));
//        plane->clone(vec3(15 , 0  , 5 ), vec3(0    , -PI/2, 0), vec3(30.0f));
//        plane->clone(vec3(-15, 0  , 5 ), vec3(0    , PI/2 , 0), vec3(30.0f));


    // Light source
//    ModelMaterial* bright = new ModelMaterial("bright");
//        bright->ambient = vec3(10.0f/4.0f);
//        bright->diffuse = vec3(0.0f);
//        bright->specular = vec3(0.0f);
//        bright->texture = Texture::get("Textures/white.png");

    new Entity("Light",
    {
//        new Graphic(Mesh::createSphere(bright, ALLFLAGS, 0.25f)),
        new Light(GE_POINT_LIGHT, vec3(0.0f), vec3(0.9f), 1.0f, 1, 0.01, 0),
        new Transform(vec3(5, 2, 4))
    });


    // Camera
    new Entity("MainCamera",
    {
//        new Camera(70, 0.1f, 100.0f, vec3(0.67f, 0.92f, 1.0f)),
        new Camera(70, 0.1f, 100.0f, vec3(0.0f)),
        new Transform(),
//        new Skybox(),

        new TPSCameraScript(player->getTransform(), 0.2f, 7.0f)
    });
}

void water()
{
    reinterpret_cast<ModelMaterial*>(Material::base)->texture = Texture::get("Textures/0.png");
    WaterMat* w = new WaterMat("water");
    GUIMaterial* guiM = new GUIMaterial("gui"); guiM->texture = Texture::get("Textures/0.png");

    // Prototype
        Entity* plane = new Entity("Walls",
        {
            new Graphic(Mesh::createCube(Material::base, ALLFLAGS, vec3(0.5f, 0.5f, 0.01f))),
            new Box(vec3(0.5f, 0.5f, 0.01f)),
            new RigidBody(0.0f)
        }, true);

    // Main entities
        Entity* player = new Entity("Player",
        {
            new Transform(vec3(0, 0, 0), vec3(0), vec3(1.0f)),

            new RigidBody(10.0f),

            new PlayerScript(plane, 10.0f, 2.0f, 30.0f)
        });

        new Entity("MainCamera",
        {
            new Camera(70, 0.1f, 100.0f, vec3(0.67f, 0.92f, 1.0f)),
            new Skybox(),

            new TPSCameraScript(player->getTransform(), 0.2f, 7.0f, vec3(0, 0, 1.0f))
        });

//        new Entity("Test",
//        {
//            new Camera(3, 0.1f, 100.0f, vec3(0.67f, 0.92f, 1.0f), false),
//
//            new TPSCameraScript(player->getTransform(), 0.2f, 7.0f)
//        });
//
        new Entity("GUI",
        {
            new Camera(70, 0.1f, 100.0f),
            new Graphic(Mesh::createQuad(guiM, VERTICES|TEXCOORDS)),
            new Transform(vec3(-0.7, 0.7, 0), vec3(0.0f), vec3(0.5f)),
            new sc(guiM)
        });

    // Water
        Entity* water = new Entity("water",
        {
            new Graphic(Mesh::createQuad(w, VERTICES)),
            new Transform(vec3(0, 0, -9.5f), vec3(0), vec3(30.0f)),

            new WaterScript(w)
        });

        new Entity("Reflect",
        {
            new Camera(70, 0.1f, 100.0f, vec3(0.67f, 0.92f, 1.0f), new RenderTexture(1280, 720))
        });

        new Entity("Refract",
        {
            new Camera(70, 0.1f, 100.0f, vec3(0.67f, 0.92f, 1.0f), new RenderTexture(640, 360))
        });

    // Scene
        Entity* ground = plane->clone(vec3(0  , 0  , -10), vec3(0), vec3(30.0f));

        plane->clone(vec3(0  , -15, 5 ), vec3(-PI/2, 0    , 0), vec3(30.0f));
        plane->clone(vec3(0  , 15 , 5 ), vec3(PI/2 , 0    , 0), vec3(30.0f));
        plane->clone(vec3(15 , 0  , 5 ), vec3(0    , -PI/2, 0), vec3(30.0f));
        plane->clone(vec3(-15, 0  , 5 ), vec3(0    , PI/2 , 0), vec3(30.0f));

        plane->clone(vec3(0, 10, -9.49f), vec3(0), vec3(30.0f, 10, 1));

    // Light source
        ModelMaterial* bright = new ModelMaterial("bright");
            bright->ambient = vec3(10.0f/4.0f);
            bright->diffuse = vec3(0.0f);
            bright->specular = vec3(0.0f);
            bright->texture = Texture::get("Textures/white.png");

        new Entity("Light",
        {
            new Graphic(Mesh::createSphere(bright, ALLFLAGS, 0.25f)),
            new Light(GE_POINT_LIGHT, vec3(0.0f), vec3(0.9f), 1.0f, 1, 0.01, 0),
            new Transform(vec3(5, 2, 8))
        });
}

void heightfield()
{
    reinterpret_cast<ModelMaterial*>(Material::base)->texture = Texture::get("Textures/0.png");
    PhysicEngine::get()->setGravity(vec3(0.0f));

    // Main entities
        Entity* player = new Entity("Player",
        {
            new Transform(vec3(0, 0, 0), vec3(0), vec3(1, 1, 1)),
            new RigidBody(10.0f),

            new PlayerScript(nullptr, 10.0f, 2.0f, 30.0f)
        });

        new Entity("MainCamera",
        {
            new Camera(70, 0.1f, 1000.0f, vec3(0.67f, 0.92f, 1.0f)),
            new Skybox(),

            new TPSCameraScript(player->getTransform(), 0.2f, 7.0f)
        });

        new Entity("Terrain",
        {
            new Graphic(new Terrain("Heightmaps/Sculptor/Sculptor.terrain")),
            new Transform(vec3(-129, -129, -120), vec3(0), vec3(1)),

            new TerrainScript("Player")
        });

    // Light source
        ModelMaterial* bright = new ModelMaterial("bright");
            bright->ambient = vec3(10.0f/4.0f);
            bright->diffuse = vec3(0.0f);
            bright->specular = vec3(0.0f);
            bright->texture = Texture::get("Textures/white.png");

        new Entity("Light",
        {
            new Graphic(Mesh::createSphere(bright, ALLFLAGS, 0.25f)),
            new Light(GE_POINT_LIGHT, vec3(0.0f), vec3(0.9f), 1.0f, 1, 0.01, 0),
            new Transform(vec3(5, 2, 8))
        });
}

void game()
{
    reinterpret_cast<ModelMaterial*>(Material::base)->texture = Texture::get("Textures/white.png");

    Entity* plane = new Entity("Walls",
    {
        new Graphic(Mesh::createCube(Material::base, ALLFLAGS, vec3(0.5f, 0.5f, 0.01f))),
        new Box(vec3(0.5f, 0.5f, 0.01f)),
        new RigidBody(0.0f)
    }, true);

    // Main entities
        Entity* player = new Entity("Player",
        {
            new Transform(vec3(0, 0, 0.4-10.0f), vec3(PI/2, 0, 0), vec3(0.01f)),
            new Graphic(new AnimatedModel("Models/Pilot/Pilot.mesh.xml")),

            new AnimScript()
        });

        new Entity("MainCamera",
        {
            new Camera(70, 0.1f, 1000.0f, vec3(0.67f, 0.92f, 1.0f)),
            new Skybox(),

            new TPSCameraScript(player->getTransform(), 0.2f, 5.0f, vec3(0, 0, 1.75))
        });

    // Water
        WaterMat* w = new WaterMat("water");
        Entity* water = new Entity("water",
        {
            new Graphic(Mesh::createQuad(w, VERTICES)),
            new Transform(vec3(0, 0, -9.5f), vec3(0), vec3(30.0f)),

            new WaterScript(w)
        });

        new Entity("Reflect",
        {
            new Camera(70, 0.1f, 100.0f, vec3(0.67f, 0.92f, 1.0f), new RenderTexture(1280, 720))
        });

        new Entity("Refract",
        {
            new Camera(70, 0.1f, 100.0f, vec3(0.67f, 0.92f, 1.0f), new RenderTexture(1280, 720))
        });

    // Scene
        Entity* ground = plane->clone(vec3(0  , 0  , -10), vec3(0), vec3(30.0f));

        plane->clone(vec3(0  , -15, 5 ), vec3(-PI/2, 0    , 0), vec3(30.0f));
        plane->clone(vec3(0  , 15 , 5 ), vec3(PI/2 , 0    , 0), vec3(30.0f));
        plane->clone(vec3(15 , 0  , 5 ), vec3(0    , -PI/2, 0), vec3(30.0f));
        plane->clone(vec3(-15, 0  , 5 ), vec3(0    , PI/2 , 0), vec3(30.0f));

        plane->clone(vec3(0, 10, -9.49f), vec3(0), vec3(30.0f, 10, 1));

    // Light source
        ModelMaterial* bright = new ModelMaterial("bright");
            bright->ambient = vec3(10.0f/4.0f);
            bright->diffuse = vec3(0.0f);
            bright->specular = vec3(0.0f);
            bright->texture = Texture::get("Textures/white.png");

        new Entity("Light",
        {
            new Graphic(Mesh::createSphere(bright, ALLFLAGS, 0.25f)),
            new Light(GE_POINT_LIGHT, vec3(0.0f), vec3(0.9f), 1.0f, 1, 0.01, 0),
            new Transform(vec3(1, 2, -7))
        });
}
