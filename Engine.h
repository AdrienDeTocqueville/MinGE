#ifndef ENGINE_H
#define ENGINE_H

#include "Utility/Input.h"
#include "Utility/Debug.h"
#include "Utility/Random.h"

#include "Systems/GraphicEngine.h"
#include "Systems/PhysicEngine.h"
#include "Systems/ScriptEngine.h"

#include "Assets/Program.h"

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
            Input* input = nullptr;

            GraphicEngine* gEngine = nullptr;
            PhysicEngine*  pEngine = nullptr;
            ScriptEngine*  sEngine = nullptr;

            sf::Event* event;
            sf::Clock clock;
            sf::Font font;
            sf::Text text;

            float acu;  unsigned frames;
            bool pause;

        /// Attributes (static)
            static Engine* instance;
};

#endif // ENGINE_H
