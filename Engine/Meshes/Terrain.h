#pragma once

#include "Assets/Mesh.h"
#include "Utility/Accel/Quadtree.h"

class Terrain : public Mesh
{
public:
	Terrain(std::string _file);
	virtual ~Terrain();

	/// Methods (public)
	void updateTree(vec3 _position);

protected:
	/// Methods (protected)
	bool loadArrays();

	void generateTexCoord();
	void generateNormals();

	/// Attributes
	QuadTree* tree;

	std::string file;

	unsigned side = 0;
};
