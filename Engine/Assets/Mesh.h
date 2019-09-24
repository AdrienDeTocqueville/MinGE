#ifndef MESH_H
#define MESH_H

#include "Systems/GraphicEngine.h"
#include "Assets/Material.h"
#include "Utility/Accel/AABB.h"

#define VERTICES  1
#define NORMALS   2
#define TEXCOORDS 4

const unsigned ALLFLAGS = (VERTICES | NORMALS | TEXCOORDS);

class Transform;

struct Submesh
{
	Submesh():  mode(GL_TRIANGLES), first(0), count(0), material(-1)
	{ }
	Submesh(GLdouble _mode, unsigned _first, unsigned _count, unsigned _material):
		mode(_mode), first(_first), count(_count), material(_material)
	{ }

	~Submesh()
	{ }

	GLdouble mode;
	unsigned first, count;

	unsigned material;
};


class Mesh
{
	public:
		Mesh(unsigned _dataFlags = ALLFLAGS);

		/// Methods (public)
			virtual void render(Transform* _tr, const std::vector<Material*>& _materials);

			void destroy();

		/// Methods (static)
			static void clear()
			{
				for(Mesh* mesh: meshes)
					delete mesh;

				meshes.clear();
			}

			static Mesh* createCube(Material* _material = Material::base, unsigned _dataFlags = ALLFLAGS, vec3 _halfExtent = vec3(0.5f));
			static Mesh* createQuad(Material* _material = Material::base, unsigned _dataFlags = ALLFLAGS, vec2 _halfExtent = vec2(0.5f));
			static Mesh* createSphere(Material* _material = Material::base, unsigned _dataFlags = ALLFLAGS, float _radius = 0.5f, unsigned _slices = 20, unsigned _stacks = 10);
			static Mesh* createCylinder(Material* _material = Material::base, unsigned _dataFlags = ALLFLAGS, float _base = 0.5f, float _top = 0.5f, float _height = 1.0f, unsigned _slices = 20);

		/// Getter
			unsigned getSubmeshesCount() const
			{ return submeshes.size(); }

			std::vector<vec3>* getVertices()
			{ return &vertices; }
			std::vector<vec3>* getNormals()
			{ return &normals; }

			unsigned getMaterialIndex(std::string _name) const;
			const std::vector<Material*>& getMaterials() const;

			AABB getAABB() const
			{ return aabb; }

		// TODO: make them private
		/// Attributes (public)
			std::vector<Submesh> submeshes;
			std::vector<Material*> materials;

			std::vector<vec3> vertices;
			std::vector<vec3> normals;
			std::vector<vec2> texCoords;

	protected:
		/// Methods (protected)
			virtual ~Mesh();

			virtual void loadBuffers();

		/// Attributes (protected)
			unsigned vbo;
			unsigned vao;

			unsigned dataFlags;
			AABB aabb;

		/// Attributes (static)
			static std::list<Mesh*> meshes;
};

#endif // MESH_H
