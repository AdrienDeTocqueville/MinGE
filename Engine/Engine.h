#ifndef ENGINE_H
#define ENGINE_H

#include "Systems/GraphicEngine.h"
#include "Systems/PhysicEngine.h"
#include "Systems/ScriptEngine.h"

class Entity;

class Engine
{
    public:
        Engine(sf::RenderWindow* _window, unsigned _FPS = 60);
        ~Engine();

        /// Methods (static)
            static Engine* get()    { return instance; }

        /// Methods (public)
            void start();

            bool update();

            void clear();

        /// Setters
            void setPause(bool _pause);
            void togglePause();
            void setWindowSize(vec2 _newSize);

        /// Getters
            bool getPause() const;

    private:
        /// Attributes
            GraphicEngine* gEngine = nullptr;
            PhysicEngine*  pEngine = nullptr;
            ScriptEngine*  sEngine = nullptr;

            sf::Event* event;
            sf::Clock clock;

            bool pause;

#ifdef REPORTFPS
            sf::Font font;
            sf::Text text;

            float acu = 0.0f;
            unsigned frames = 0;
#endif

        /// Attributes (static)
            static Engine* instance;
};

#endif // ENGINE_H
