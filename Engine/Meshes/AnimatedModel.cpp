#include "Meshes/AnimatedModel.h"
#include "Utility/tinyxml2.h"

#include <fstream>
#include <glm/gtx/transform.hpp>

#define VEC3_XML(e) vec3(e->FloatAttribute("x"), e->FloatAttribute("y"), e->FloatAttribute("z"))

using namespace tinyxml2;

AnimatedModel::AnimatedModel(std::string _mesh):
	Mesh(),
	root(nullptr),
	ibo(0),
	mesh(_mesh)
{
	std::size_t found = mesh.rfind("/");
	if (found != std::string::npos)
		skeleton = mesh.substr(0, found+1);

	found = mesh.find(".");
	if (found != std::string::npos)
		material = mesh.substr(0, found) + ".material";

	if (load_material() && load_mesh_xml() && load_skeleton_xml())
		loadBuffers();
}

AnimatedModel::~AnimatedModel()
{
	glDeleteBuffers(1, &ibo);
}

/// Methods (public)
void AnimatedModel::render(Transform* _tr, const std::vector<Material*>& _materials)
{
	if (vao && ibo)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBindVertexArray(vao);

		for (Submesh& submesh: submeshes)
			if (_materials[submesh.material]->use(_tr))
				glDrawElements(submesh.mode, submesh.count, GL_UNSIGNED_SHORT, BUFFER_OFFSET(submesh.first*sizeof(unsigned short)));
	}
}

/// Methods (protected)
void AnimatedModel::loadBuffers()
{
	unsigned dataSize[5];
	dataSize[0] = vertices.size() *sizeof(vec3);
	dataSize[1] = normals.size()  *sizeof(vec3);
	dataSize[2] = texCoords.size()*sizeof(vec2);
	dataSize[3] = boneIds.size()  *sizeof(ivec4);
	dataSize[4] = weights.size()  *sizeof(vec4);

	unsigned offset[5];
	offset[0] = dataSize[0];
	offset[1] = dataSize[1] + offset[0];
	offset[2] = dataSize[2] + offset[1];
	offset[3] = dataSize[3] + offset[2];
	offset[4] = dataSize[4] + offset[3];

	/// VBO
	glDeleteBuffers(1, &vbo);
	glGenBuffers(1, &vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glBufferData(GL_ARRAY_BUFFER, offset[4], nullptr, GL_STATIC_DRAW);

			glBufferSubData(GL_ARRAY_BUFFER, 0		, dataSize[0], &vertices [0]);
			glBufferSubData(GL_ARRAY_BUFFER, offset[0], dataSize[1], &normals  [0]);
			glBufferSubData(GL_ARRAY_BUFFER, offset[1], dataSize[2], &texCoords[0]);
			glBufferSubData(GL_ARRAY_BUFFER, offset[2], dataSize[3], &boneIds  [0]);
			glBufferSubData(GL_ARRAY_BUFFER, offset[3], dataSize[4], &weights  [0]);

	/// VAO
	glDeleteVertexArrays(1, &vao);
	glGenVertexArrays(1, &vao);

	glBindVertexArray(vao);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offset[0]));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offset[1]));
		glVertexAttribIPointer(3, 4, GL_INT , 0, BUFFER_OFFSET(offset[2]));
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offset[3]));

	/// IBO
	glDeleteBuffers(1, &ibo);
	glGenBuffers(1, &ibo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);
}

bool AnimatedModel::load_mesh_xml()
{
	std::string path = "Resources/" + mesh;

	XMLDocument doc;
	if (doc.LoadFile(path.c_str()) != XML_SUCCESS)
	{
		Error::add(FILE_NOT_FOUND, "AnimatedModel::load_mesh_xml() -> " + path);
		return false;
	}


	XMLElement* meshElement= doc.FirstChildElement("mesh");

	skeleton += std::string(meshElement->FirstChildElement("skeletonlink")->Attribute("name")) + ".xml";


	XMLElement* submeshesElement = meshElement->FirstChildElement("submeshes");

	XMLElement* submesh = submeshesElement->FirstChildElement("submesh");
	while (submesh)
	{
		unsigned firstVertex = vertices.size();
		unsigned firstIndex = indices.size();

		/// GEOMETRY
		XMLElement* geometry = submesh->FirstChildElement("geometry");

		int vertexCount = geometry->IntAttribute("vertexcount");
		 vertices.reserve(firstVertex + vertexCount);
		  normals.reserve(firstVertex + vertexCount);
		texCoords.reserve(firstVertex + vertexCount);

		XMLElement* vb = geometry->FirstChildElement("vertexbuffer");
		while (vb)
		{
			bool isPositions = vb->BoolAttribute("positions");
			bool   isNormals = vb->BoolAttribute("normals");
			int  isTexCoords = vb->IntAttribute("texture_coords");

			XMLElement* vertex = vb->FirstChildElement("vertex");
			while (vertex)
			{
				if (isPositions)
					vertices.push_back( VEC3_XML(vertex->FirstChildElement("position")) );

				if (isNormals)
					normals.push_back( VEC3_XML(vertex->FirstChildElement("normal")) );

				if (isTexCoords)
				{
					auto t = vertex->FirstChildElement("texcoord");
					texCoords.push_back( vec2(t->FloatAttribute("u"), 1-t->FloatAttribute("v")) );
				}

				vertex = vertex->NextSiblingElement("vertex");
			}

			vb = vb->NextSiblingElement("vertexbuffer");
		}


		/// FACES
		XMLElement* faces = submesh->FirstChildElement("faces");

		int faceCount = faces->IntAttribute("count");
		indices.reserve(indices.size() + 3.0f*faceCount);

		XMLElement* face = faces->FirstChildElement("face");
		const std::string attributes[3] = {"v1", "v2", "v3"};
		while (face)
		{
			for (int i(0) ; i < 3 ; i++)
				indices.push_back( firstVertex + face->IntAttribute(attributes[i].c_str()) );

			face = face->NextSiblingElement("face");
		}


		/// BONE ASSIGNEMENTS
		boneIds.resize(vertices.size(), ivec4(0));
		weights.resize(vertices.size(), vec4(0.0f));

		XMLElement* ba = submesh->FirstChildElement("boneassignments");
		XMLElement* vba = ba->FirstChildElement("vertexboneassignment");
		while (vba)
		{
			unsigned vi = vba->IntAttribute("vertexindex");
			int   boneId = vba->IntAttribute("boneindex");
			float weight = vba->FloatAttribute("weight");

			unsigned i(0);
			for (; i < 4 ; i++)
				if (weights[firstVertex + vi][i] == 0.0f)
					break;

			if (i == 4) i--;

			boneIds[firstVertex + vi][i] = boneId;
			weights[firstVertex + vi][i] = weight;

			vba = vba->NextSiblingElement("vertexboneassignment");
		}

		std::string materialName = submesh->Attribute("material");
		std::string dataType = submesh->Attribute("operationtype");

		GLdouble mode = GL_QUADS;
		if (dataType == "triangle_list")
			mode = GL_TRIANGLES;

		submeshes.push_back( Submesh(mode, firstIndex, indices.size()-firstIndex, getMaterialIndex(materialName)) );

		submesh = submesh->NextSiblingElement("submesh");
	}

	return true;
}

bool AnimatedModel::load_skeleton_xml()
{
	std::string path = "Resources/" + skeleton;

	XMLDocument doc;
	if (doc.LoadFile(path.c_str()) != XML_SUCCESS)
	{
		Error::add(FILE_NOT_FOUND, "AnimatedModel::load_skeleton_xml() -> " + path);
		return false;
	}


	XMLElement* skeletonElement = doc.FirstChildElement("skeleton");

	/// BONES
	XMLElement* bonesElement = skeletonElement->FirstChildElement("bones");

	XMLElement* bone = bonesElement->FirstChildElement("bone");
	while (bone)
	{
		std::string name = bone->Attribute("name");

		vec3 position( VEC3_XML(bone->FirstChildElement("position")) );

		XMLElement* rotation  = bone->FirstChildElement("rotation");

		float angle = rotation->FloatAttribute("angle");
		vec3 axis( VEC3_XML(rotation->FirstChildElement("axis")) );

		bones.push_back( Bone(name, position, angle, axis) );

		bone = bone->NextSiblingElement("bone");
	}

	/// BONE HIERARCHY
	XMLElement* boneHierarchy = skeletonElement->FirstChildElement("bonehierarchy");

	XMLElement* boneparent = boneHierarchy->FirstChildElement("boneparent");
	while (boneparent)
	{
		std::string boneName   = boneparent->Attribute("bone");
		std::string parentName = boneparent->Attribute("parent");

		unsigned boneIndex = getBoneIndex(boneName);
		unsigned parentIndex = getBoneIndex(parentName);

		bones[boneIndex].parent = parentIndex;
		bones[parentIndex].children.push_back(boneIndex);

		boneparent = boneparent->NextSiblingElement("boneparent");
	}

	for (unsigned i(0) ; i < bones.size() ; i++)
	{
		if (bones[i].parent == -1)
		{
			root = &bones[i];
			break;
		}
	}

	computeMatrix(root);

	for (unsigned i(0) ; i < bones.size() ; i++)
		bones[i].inverseBindMatrix = inverse(bones[i].inverseBindMatrix);

	/// ANIMATIONS
	XMLElement* animation = skeletonElement->FirstChildElement("animations")->FirstChildElement("animation");
	while (animation)
	{
		std::string name = animation->Attribute("name");
		float duration = animation->FloatAttribute("length");

		animations.push_back( Animation(name, duration) );
		Animation& anim = animations.back();

		/// TRACKS
		XMLElement* trackElement = animation->FirstChildElement("tracks")->FirstChildElement("track");
		while (trackElement)
		{
			anim.tracks.push_back( Track(getBoneIndex(trackElement->Attribute("bone"))) );
			Track& track = anim.tracks.back();

			/// KEYFRAMES
			XMLElement* keyframe = trackElement->FirstChildElement("keyframes")->FirstChildElement("keyframe");
			while (keyframe)
			{
				float keyTime = keyframe->FloatAttribute("time");
				vec3 keyTr = VEC3_XML(keyframe->FirstChildElement("translate"));

				XMLElement* rotation  = keyframe->FirstChildElement("rotate");
				float keyAngle = rotation->FloatAttribute("angle");
				vec3 keyAxis( VEC3_XML(rotation->FirstChildElement("axis")) );

				track.translations.push_back( std::make_pair(keyTime, keyTr) );
				track.rotations   .push_back( std::make_pair(keyTime, angleAxis(keyAngle, keyAxis)) );

				keyframe = keyframe->NextSiblingElement("keyframe");
			}

			reduceKeyframes(track);

			trackElement = trackElement->NextSiblingElement("track");
		}

		animation = animation->NextSiblingElement("animation");
	}

	return true;
}

bool AnimatedModel::load_material()
{
	std::string path = "Resources/" + material;
	std::ifstream data((path).c_str());

	if (!data)
	{
		Error::add(FILE_NOT_FOUND, "AnimatedModel::load_material() -> " + path);
		return false;
	}

	std::istringstream stream;

	std::string line, element;
	AnimatedModelMaterial* mat = nullptr;

	while (getline(data, line))
	{
		if (!line.size())
			continue;

		stream.clear();
		stream.str(line);
		stream >> element;


		if (element == "{" || element == "}")
			continue;
		else if (element == "material")
		{
			stream >> element;

			mat = new AnimatedModelMaterial(element);
			materials.push_back(mat);
		}
		else if (element == "ambient")
		{
			for (unsigned i(0) ; i < 3 ; i++)
				stream >> mat->ambient[i];
		}
		else if (element == "diffuse")
		{
			for (unsigned i(0) ; i < 3 ; i++)
			 stream >> mat->diffuse[i];
		}
		else if (element == "specular")
		{
			for (unsigned i(0) ; i < 3 ; i++)
			 stream >> mat->specular[i];
		}
		else if (element == "shininess_exponent")
		{
			stream >> mat->exponent;
		}
		else if (element == "texture")
		{
			stream >> element;
			mat->texture = Texture::get(element);
		}
	}

	return true;
}

void AnimatedModel::reduceKeyframes(Track& _track)
{
	unsigned current;

	// Translations
	std::vector<std::pair<float, vec3>> t; t.reserve(_track.translations.size());
	t.push_back( _track.translations[0] );
	current = 0;

	for (unsigned i(1) ; i < _track.translations.size() ; i++)
	{
		if (epsilonEqual(_track.translations[i].second, _track.translations[current].second, 0.01f) && i != _track.translations.size() - 1)
			continue;

		t.push_back( _track.translations[i] );
		current = i;
	}
	_track.translations = t;

	// Rotations
	std::vector<std::pair<float, quat>> r; r.reserve(_track.rotations.size());
	r.push_back( _track.rotations[0] );
	current = 0;

	for (unsigned i(1) ; i < _track.rotations.size() ; i++)
	{
		if (epsilonEqual(_track.rotations[i].second, _track.rotations[current].second, 0.01f) && i != _track.rotations.size() - 1)
			continue;

		r.push_back( _track.rotations[i] );
		current = i;
	}
	_track.rotations = r;
}

/// Methods (private)
unsigned AnimatedModel::getBoneIndex(std::string _name)
{
	for (unsigned i(0) ; i < bones.size() ; i++)
		if (bones[i].name == _name)
			return i;

	return -1;
}

/// Getters
unsigned AnimatedModel::getAnimationIndex(std::string _name)
{
	for (unsigned i(0) ; i < animations.size() ; i++)
		if (animations[i].name == _name)
			return i;

	return -1;
}

void AnimatedModel::computeMatrix(Bone* _bone)
{
	if (_bone->parent == -1)
		_bone->inverseBindMatrix = mat4(1.0f);
	else
		_bone->inverseBindMatrix = bones[_bone->parent].inverseBindMatrix;


	_bone->inverseBindMatrix *= glm::translate(_bone->position);
	_bone->inverseBindMatrix *= mat4_cast(_bone->rotation);

	for (unsigned i(0) ; i < _bone->children.size() ; i++)
		computeMatrix( &bones[_bone->children[i]] );
}
