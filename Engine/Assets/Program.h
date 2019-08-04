#ifndef PROGRAM_H
#define PROGRAM_H

#include "includes.h"

class Program
{
    class Shader;

    friend class Engine;
    friend class GraphicEngine;

    public:
        Program(std::string& _vertex, std::string& _fragment);
        Program(Shader* _vertex, Shader* _fragment);

		/// Methods (static)
            static Program* get(std::string _vertex, std::string _fragment);
            static void clear();

		/// Methods (public)
            void use();

            unsigned getLocation(const std::string& _name) const;
            void addLocation(unsigned _location);

            void send(unsigned _location, int _value) const;
            void send(unsigned _location, unsigned _value) const;
            void send(unsigned _location, float _value) const;
            void send(unsigned _location, vec3 _value) const;
            void send(unsigned _location, vec4 _value) const;
            void send(unsigned _location, mat3 _value) const;
            void send(unsigned _location, mat4 _value) const;
            void send(unsigned _location, const std::vector<mat4>& _values) const;

    private:
        /// Destructor (private)
            ~Program();

		/// Methods (private)
            void linkProgram();

        /// Attributes (static)
            static std::vector<Program*> programs;
            static unsigned current;

        /// Attributes
            unsigned program;

			std::vector<unsigned> locations;

            Shader *vertex, *fragment;

            class Shader
            {
                friend class Program;

                public:
                    static Shader* get(std::string& _shader);
                    static void clear();
                private:
                    static Shader* load(std::string& _shader);

                    static std::unordered_map<std::string, Shader*> shaders;
                    static std::unordered_map<std::string, Shader*>::iterator it;

                    Shader(unsigned _shader): shader(_shader)
                    { }

                    ~Shader()
                    {
                        glDeleteShader(shader);
                    }

                    unsigned shader;
                    std::vector<std::string> uniforms;


                    std::string getVarName(std::string str);
            };
};

#endif // PROGRAM_H
