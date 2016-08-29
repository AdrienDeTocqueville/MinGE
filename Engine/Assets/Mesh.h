#ifndef MESH_H
#define MESH_H

#include "Systems/GraphicEngine.h"
#include "Assets/Material.h"

#define VERTICES  1
#define NORMALS   2
#define TEXCOORDS 4

const unsigned ALLFLAGS = (VERTICES | NORMALS | TEXCOORDS);

class Transform;

struct Submesh
{
    Submesh():  mode(GL_TRIANGLES), first(0), count(0), material(nullptr)
    { }
    Submesh(GLdouble _mode, unsigned _first, unsigned _count, Material* _material):
        mode(_mode), first(_first), count(_count), material(_material)
    { }

    ~Submesh()
    { }

    GLdouble mode;
    unsigned first, count;

    Material* material;
};


class Mesh
{
    public:
        Mesh(unsigned _dataFlags = ALLFLAGS);
        virtual ~Mesh();

        /// Methods (public)
            virtual void render(Transform* _tr);

        /// Methods (static)
            static void clear()
            {
                for (unsigned i(0) ; i < meshes.size() ; i++)
                    delete meshes[i];

                meshes.clear();
            }

            static Mesh* createCube(Material* _material = Material::base, unsigned _dataFlags = ALLFLAGS, vec3 _halfExtent = vec3(0.5f));
            static Mesh* createQuad(Material* _material = Material::base, unsigned _dataFlags = ALLFLAGS, vec2 _halfExtent = vec2(0.5f));
            static Mesh* createSphere(Material* _material = Material::base, unsigned _dataFlags = ALLFLAGS, float _radius = 0.5f, unsigned _slices = 20, unsigned _stacks = 10);
            static Mesh* createCylinder(Material* _material = Material::base, unsigned _dataFlags = ALLFLAGS, float _base = 0.5f, float _top = 0.5f, float _height = 1.0f, unsigned _slices = 20);

        /// Getter
            unsigned getSubmeshesCount()
            { return submeshes.size(); }

            std::vector<vec3>* getVertices()
            { return &vertices; }
            std::vector<vec3>* getNormals()
            { return &normals; }

        /// Attributes (public)
            std::vector<Submesh> submeshes;

            std::vector<vec3> vertices;
            std::vector<vec3> normals;
            std::vector<vec2> texCoords;

    protected:
        /// Methods (protected)
            virtual void loadBuffers();

        /// Attributes (protected)
            unsigned vbo;
            unsigned vao;

            unsigned dataFlags;

        /// Attributes (static)
            static std::vector<Mesh*> meshes;
};

#endif // MESH_H
