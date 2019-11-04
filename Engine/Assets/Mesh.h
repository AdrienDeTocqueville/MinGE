#pragma once

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
	Submesh() { }
	Submesh(GLdouble _mode, unsigned _first, unsigned _count):
		mode(_mode), first(_first), count(_count) { }

	void draw() const;

	GLdouble mode;
	unsigned first, count;
};

typedef std::shared_ptr<class Mesh> MeshRef;

class Mesh
{
	friend class Graphic;

public:
	virtual ~Mesh();

	/// Getters
	std::vector<vec3>* getVertices()
	{ return &vertices; }
	std::vector<vec3>* getNormals()
	{ return &normals; }

	AABB getAABB() const
	{ return aabb; }

	/// Methods (static)
	static MeshRef createCube(unsigned _dataFlags = ALLFLAGS, vec3 _halfExtent = vec3(0.5f));
	static MeshRef createQuad(unsigned _dataFlags = ALLFLAGS, vec2 _halfExtent = vec2(0.5f));
	static MeshRef createSphere(unsigned _dataFlags = ALLFLAGS, float _radius = 0.5f, unsigned _slices = 20, unsigned _stacks = 10);
	static MeshRef createCylinder(unsigned _dataFlags = ALLFLAGS, float _base = 0.5f, float _top = 0.5f, float _height = 1.0f, unsigned _slices = 20);

protected:
	/// Methods (protected)
	Mesh(unsigned _dataFlags = ALLFLAGS);
	virtual void loadBuffers();

	/// Attributes (protected)
	std::vector<Submesh> submeshes;

	std::vector<vec3> vertices;
	std::vector<vec3> normals;
	std::vector<vec2> texCoords;
	//std::vector<uvec3> indices;

	unsigned vbo;
	unsigned vao;
	//unsigned ebo;

	unsigned dataFlags;
	AABB aabb;
};
