#pragma once

#include <memory>

#include "Utility/helpers.h"
#include "Utility/Accel/AABB.h"

#define VERTICES  1
#define NORMALS   2
#define TEXCOORDS 4

const unsigned ALLFLAGS = (VERTICES | NORMALS | TEXCOORDS);

class Transform;

struct Submesh
{
	Submesh(uint32_t _mode, unsigned _count, uint32_t _first_index = 0):
		mode(_mode), count(_count), offset(_first_index * sizeof(uint16_t)) { }

	const uint32_t mode;
	const uint32_t count;
	const uint32_t offset;
};

struct Vertex
{
	vec3 pos;
	vec3 normal;
	vec2 texCoords;
};

typedef std::shared_ptr<class Mesh> MeshRef;

class Mesh
{
public:
	Mesh(class aiMesh *mesh);
	~Mesh();

	/// Getters
	AABB getAABB() const
	{ return aabb; }
	unsigned getVAO() const
	{ return vao; }
	const std::vector<Submesh> &getSubmeshes() const
	{ return submeshes; }

	/// Methods (static)
	static MeshRef createCube(unsigned _dataFlags = ALLFLAGS, vec3 _halfExtent = vec3(0.5f));
	static MeshRef createQuad(unsigned _dataFlags = ALLFLAGS, vec2 _halfExtent = vec2(0.5f));
	static MeshRef createSphere(unsigned _dataFlags = ALLFLAGS, float _radius = 0.5f, unsigned _slices = 20, unsigned _stacks = 10);
	static MeshRef createCylinder(unsigned _dataFlags = ALLFLAGS, float _base = 0.5f, float _top = 0.5f, float _height = 1.0f, unsigned _slices = 20);

	static MeshRef load(std::string _file);

protected:
	/// Methods (protected)
	Mesh(unsigned _dataFlags = ALLFLAGS);
	virtual void loadBuffers();

	/// Attributes (protected)
	std::vector<Submesh> submeshes;

	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;

	unsigned vao, vbo, ebo;

	unsigned dataFlags;
	AABB aabb;
};
