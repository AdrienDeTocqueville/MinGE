#include "Utility/Accel/Quadtree.h"

/// Quadtree
QuadTree::QuadTree() :
	ibo(GL::GenBuffer()), size(0), prevPos(vec2(-1, -1))
{ }

QuadTree::~QuadTree()
{
	GL::DeleteBuffer(ibo);
	delete root;
}

void QuadTree::init(std::vector<vec3>* _vertices, std::vector<vec3>* _normals, unsigned _side)
{
	resize(_side*_side*2);

	maxDepth = log10(_side-1) / log10(2);

	dist.resize(maxDepth);

	for (unsigned i(0) ; i < maxDepth ; i++)
		dist[i] = pow(2, (maxDepth - (i+1)));

	Node::treeMaxDepth = maxDepth;
	root = new Node(_vertices, _normals,  vec2(0, 0), _side, _side-1, 0);
	root->depth = 1;
}

void QuadTree::resize(unsigned _size)
{
	size = _size;

	GL::BindElementBuffer(ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size*sizeof(unsigned), 0, GL_STREAM_DRAW);
}

void QuadTree::update(vec2 _pos)
{
	vec2 pos = vec2(((int)_pos.x/step)*step, ((int)_pos.y/step)*step);

	if (pos == prevPos)
		return;

	prevPos = pos;

	indices.clear();

	Node::treeLowest = lowest;
	Node::treeMaxDepth = maxDepth;
	Node::dist = &dist;
	Node::treeIndices = &indices;

	root->update(pos);
//	root->getNormal(pos);
	root->loadIndices();

	if (indices.size() > size)
		resize(indices.size());

	GL::BindElementBuffer(ibo);
	void* adress = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
		memcpy(adress, indices.data(), indices.size()*sizeof(unsigned));
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
}

/// Node
vec2 Node::pos[4] = {vec2(0), vec2(1, 0), vec2(0, 1), vec2(1)};
void (*Node::draw[5])(Node*) = {leaf, left, right, top, bottom};
unsigned Node::treeLowest = 0, Node::treeMaxDepth = 0;
std::vector<unsigned>* Node::dist;
std::vector<unsigned>* Node::treeIndices;

Node::Node(std::vector<vec3>* _vertices, std::vector<vec3>* _normals, vec2 _pos, unsigned _treeSide, unsigned _side, unsigned _depth) :
	depth(_depth), type(DIVIDE), normal(0.0f)
{
	nIndices[0] = _pos.x + _treeSide * _pos.y;
	nIndices[1] = nIndices[0] +_side;
	nIndices[2] = _pos.x + _treeSide * (_pos.y +_side);
	nIndices[3] = nIndices[2] +_side;

	unsigned cSide = _side/2;

	center = vec2((*_vertices)[(nIndices[0] +cSide) + _treeSide * cSide]);

	if (depth == treeMaxDepth)
	{
		type = LEAF;
		for (unsigned i(0) ; i < 4 ; i++)
		{
			childs[i] = nullptr;
			normal += (*_normals)[nIndices[i]];
		}

		normal *= 0.25;

		return;
	}

	childs[0] = new Node(_vertices, _normals, _pos+vec2(0	, 0	), _treeSide, cSide, depth+1);
	childs[1] = new Node(_vertices, _normals, _pos+vec2(cSide, 0	), _treeSide, cSide, depth+1);
	childs[2] = new Node(_vertices, _normals, _pos+vec2(0	, cSide), _treeSide, cSide, depth+1);
	childs[3] = new Node(_vertices, _normals, _pos+vec2(cSide, cSide), _treeSide, cSide, depth+1);
}

Node::~Node()
{
	for (unsigned i(0) ; i != 4 ; i++)
		if (childs[i] != nullptr)
			delete childs[i];
}

bool inSquare(vec2 point, vec2 center, int side)
{
	if (point.x >= center.x - side)
		if (point.x <= center.x + side)
			if (point.y >= center.y - side)
				if (point.y <= center.y + side)
					return true;

	return false;
}

vec3 Node::getNormal(vec2 _pos)
{
	if (childs[0] == nullptr)
		return normal;

	type = DIVIDE;

	vec3 normals[4];
	vec3 moyenne(0.0f);

	for (unsigned i(0) ; i < 4 ; i++)
	{
		normals[i] = childs[i]->getNormal(_pos);

		moyenne += normals[i];
	}

	moyenne *= 0.25f;

//	std::cout << "m: " << moyenne.x << " " << moyenne.y << " " << moyenne.z << std::endl;

	/// Non optimal
	float epsilon = length(center - _pos) / sqrt(2 * 257 * 257);

//	std::cout << center.x << "   " << center.y << "   " << _pos.x << "   " << _pos.y << std::endl;
//	std::cout << 1.0f-epsilon << std::endl;

	for (unsigned i(0) ; i < 4 ; i++)
		if (dot(normals[i], moyenne) < 1.0f - epsilon)
			return vec3(0.0f);


	type = LEAF;
	return moyenne;
}

void Node::update(vec2 _pos)
{
	type = LEAF;

	if ( inSquare(center, _pos, (*dist)[depth-1] * ((treeLowest-1)*(treeMaxDepth - depth+1) + 1)) )
	{
		int dX = abs(center.x - _pos.x);
		int dY = abs(center.y - _pos.y);

		if (dX == dY)   /// Dans un angle
			type = LEAF;
		else if (dX > dY)
		{
			if (_pos.x > center.x)
				type = LEFT;
			else
				type = RIGHT;
		}
		else /// if (dX < dY)
		{
			if (_pos.y > center.y)
				type = BOTTOM;
			else
				type = TOP;
		}

	}

	if (depth >= treeLowest)
		if ( !inSquare(center, _pos, (*dist)[depth-1] * (treeLowest-1)*(treeMaxDepth - depth+1)) )
			return;

	type = DIVIDE;

	if (depth == treeMaxDepth-1)
		return;

	for (unsigned i(0) ; i != 4 ; i++)
		childs[i]->update(_pos);
}

void Node::loadIndices()
{
	if (type == DIVIDE)
		for (int i(0) ; i < 4 ; i++)
			childs[i]->loadIndices();
	else
//		leaf(this);
		(*draw[type])(this);
}

void Node::leaf(Node* _node)
{
	treeIndices->push_back(_node->nIndices[0]);
	treeIndices->push_back(_node->nIndices[1]);
	treeIndices->push_back(_node->nIndices[3]);

	treeIndices->push_back(_node->nIndices[0]);
	treeIndices->push_back(_node->nIndices[3]);
	treeIndices->push_back(_node->nIndices[2]);
}

void Node::left(Node* _node)
{
	treeIndices->push_back(_node->nIndices[0]);
	treeIndices->push_back(_node->nIndices[1]);
	treeIndices->push_back(_node->childs[1]->nIndices[3]);

	treeIndices->push_back(_node->nIndices[3]);
	treeIndices->push_back(_node->nIndices[2]);
	treeIndices->push_back(_node->childs[1]->nIndices[3]);

	treeIndices->push_back(_node->nIndices[0]);
	treeIndices->push_back(_node->childs[1]->nIndices[3]);
	treeIndices->push_back(_node->nIndices[2]);
}

void Node::right(Node* _node)
{
	treeIndices->push_back(_node->nIndices[0]);
	treeIndices->push_back(_node->nIndices[1]);
	treeIndices->push_back(_node->childs[0]->nIndices[2]);

	treeIndices->push_back(_node->nIndices[3]);
	treeIndices->push_back(_node->nIndices[2]);
	treeIndices->push_back(_node->childs[0]->nIndices[2]);

	treeIndices->push_back(_node->nIndices[1]);
	treeIndices->push_back(_node->nIndices[3]);
	treeIndices->push_back(_node->childs[0]->nIndices[2]);
}

void Node::top(Node* _node)
{
	treeIndices->push_back(_node->nIndices[2]);
	treeIndices->push_back(_node->nIndices[0]);
	treeIndices->push_back(_node->childs[0]->nIndices[1]);

	treeIndices->push_back(_node->nIndices[1]);
	treeIndices->push_back(_node->nIndices[3]);
	treeIndices->push_back(_node->childs[0]->nIndices[1]);

	treeIndices->push_back(_node->nIndices[3]);
	treeIndices->push_back(_node->nIndices[2]);
	treeIndices->push_back(_node->childs[0]->nIndices[1]);
}

void Node::bottom(Node* _node)
{
	treeIndices->push_back(_node->nIndices[1]);
	treeIndices->push_back(_node->nIndices[3]);
	treeIndices->push_back(_node->childs[2]->nIndices[3]);

	treeIndices->push_back(_node->nIndices[2]);
	treeIndices->push_back(_node->nIndices[0]);
	treeIndices->push_back(_node->childs[2]->nIndices[3]);

	treeIndices->push_back(_node->nIndices[0]);
	treeIndices->push_back(_node->nIndices[1]);
	treeIndices->push_back(_node->childs[2]->nIndices[3]);
}
