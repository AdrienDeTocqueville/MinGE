#include "Meshes/Terrain.h"
#include "Utility/Error.h"

#include <fstream>

Terrain::Terrain(std::string _file):
	file(_file)
{
	tree = new QuadTree();

	if (loadArrays())
	{
		loadBuffers();

		tree->init(&vertices, &normals, side);
		tree->setLowest(5);
		tree->update( vec2(0.0f) );
	}
}

Terrain::~Terrain()
{
	delete tree;
}

/// Methods (public)
void Terrain::updateTree(vec3 _position)
{
	tree->update( vec2(_position.x, _position.y) );
}

void Terrain::render(Transform* _tr, const std::vector<Material*>& _materials)
{
	glBindVertexArray(vao);

	if (_materials[submeshes[0].material]->use(_tr))
		tree->draw();
}

/// Methods (protected)
bool Terrain::loadArrays()
{
	std::string path("Resources/" + file);

	std::ifstream field(path.c_str());

	if (!field)
	{
		Error::add(FILE_NOT_FOUND, "Terrain::loadArrays() -> " + path);
		return false;
	}

	std::istringstream stream;

	std::string line, element;

	sf::Image image;
	unsigned detailCount = 0;

	TerrainMaterial* mat = new TerrainMaterial(file);   // Every Terrain get a different material name
	materials.push_back(mat);
	submeshes.emplace_back();
	submeshes[0].material = 0;

	while (getline(field, line))
	{
		if (!line.size())
			continue;

		stream.clear();
		stream.str(line);
		stream >> element;

		if (element == "Heightmap")
		{
			stream >> element;

			if (!image.loadFromFile("Resources/"+element))
			{
				Error::add(FILE_NOT_FOUND, "Terrain::loadArrays() -> Resources/" +element);
				image.create(257, 257);
			}

			image.flipVertically();
			mat->side = side = min(image.getSize().x, image.getSize().y);
		}
		else if (element == "Colormap")
		{
			stream >> element;
			mat->textures[0] = Texture::get(element);
		}
		else if (element == "Detailmap")
		{
			stream >> element;
			mat->textures[1] = Texture::get(element);
		}
		else if (element == "DetailTexture")
		{
			stream >> element >> mat->detailScale[detailCount];
			mat->textures[2+detailCount++] = Texture::get(element);
		}
		else
		{
			Error::add(MINGE_ERROR, "Terrain::loadArrays() -> Unknown entry: " +element);
		}
	}

	for (unsigned i(0) ; i < TerrainMaterial::TEX_COUNT ; i++)
	{
		if (mat->textures[i] == nullptr)
		{
			Error::add(MINGE_ERROR, "Terrain::loadArrays() -> Texture uninitializated at index: " + toString<unsigned>(i));
		}
	}

	mat->detailScale = (float)side / mat->detailScale;


	/// Load vertices array
		vertices.resize(side*side);

		for (unsigned y(0) ; y < side ; y++)
		for (unsigned x(0) ; x < side ; x++)
			vertices[x + y*side] = vec3(x, y, (float)image.getPixel(x, y).r);

	/// Smooth
		for (unsigned x(0) ; x < side ; x++)
		for (unsigned y(0) ; y < side ; y++)
		{
			if (x != 0 && x != side-1)
				vertices[x+ side*y] = (vertices[(x-1)+ side*y] + vertices[x+ side*y] + vertices[(x+1)+ side*y]) /3.0f;
			if (y != 0 && y != side-1)
				vertices[x+ side*y] = (vertices[x+ side*(y-1)] + vertices[x+ side*y] + vertices[x+ side*(y+1)]) /3.0f;
		}

	/// Load normals array
		generateNormals();
	/// Load texture coordinates array
		generateTexCoord();

	return true;
}

void Terrain::generateNormals()
{
	normals.resize(side*side);

	for (unsigned y(0) ; y < side ; y++)
	for (unsigned x(0) ; x < side ; x++)
	{
		unsigned _x(min(x, side-2)), _y(min(y, side-2));

		vec3 u = vertices[(_x+1) + _y*side] - vertices[_x + _y*side];
		vec3 v = vertices[_x + (_y+1)*side] - vertices[_x + _y*side];

		normals[x +y*side] = normalize(cross(u, v));
	}
}

void Terrain::generateTexCoord()
{
	texCoords.resize(side*side);

	unsigned i(0);

	for (float x(0) ; x < side ; x++)
	for (float y(0) ; y < side ; y++)
		texCoords[i++] = vec2(x/side, y/side);
}
