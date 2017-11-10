#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include "includes.h"

class Entity;
class Script;

class ContactConstraint;

class ScriptEngine
{
    friend class Engine;

    public:
        /// Methods (static)
            static ScriptEngine* get();

		/// Methods (public)
            void addComponent(Script* _script);
            void removeComponent(const Script* _script);

            void start();
            void update();
            void lateUpdate() const;

    private:
        /// Methods (private)
            ScriptEngine();
            ~ScriptEngine();

            void clear();

            static void create();
            static void destroy();

        /// Attributes (private)
            std::vector<Script*> started;
            std::vector<Script*> components;

        /// Attributes (static)
            static ScriptEngine* instance;
};

#endif // SCRIPTENGINE_H
