#ifndef STRUCTS_H
#define STRUCTS_H

#include "Systems/GraphicEngine.h"

/// Left-Up, Right-Up, Right-Down, Left-Down

enum TYPE{LEAF, LEFT, RIGHT, TOP, BOTTOM, DIVIDE};

bool inSquare(vec2 point, vec2 center, int side);

struct Node
{
    friend struct QuadTree;

    public:
        Node(std::vector<vec3>* _vertices, std::vector<vec3>* _normals, vec2 _pos, unsigned _treeSide, unsigned _side, unsigned _depth);
        ~Node();

        vec3 getNormal(vec2 _pos);
        void update(vec2 _pos);

        void loadIndices();
            static void leaf    (Node* _node);
            static void left    (Node* _node);
            static void right   (Node* _node);
            static void top     (Node* _node);
            static void bottom  (Node* _node);

    protected:
        unsigned depth;
        vec2 center;
        TYPE type;

        Node* childs[4] = { nullptr };
        unsigned nIndices[4] = { 0 }; // node Indices
        vec3 normal;

        static vec2 pos[4];
        static void (*draw[5])(Node*);
        static unsigned treeLowest, treeMaxDepth;
        static std::vector<unsigned>* dist;
        static std::vector<unsigned>* treeIndices;
};

struct QuadTree
{
    friend struct Node;

    public:
        QuadTree();
        ~QuadTree();

        void init(std::vector<vec3>* _vertices, std::vector<vec3>* _normals, unsigned _side);
        void setLowest(unsigned _lowest)
        {
            lowest = max(min(_lowest, maxDepth), (unsigned)1);
            step = pow(2, maxDepth - lowest);
        }

        void update(vec2 _pos);

        void draw()
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
                glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, BUFFER_OFFSET(0));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

    private:
    	void resize(unsigned _size);

        Node* root = nullptr;
        unsigned maxDepth = 0;
        unsigned lowest = 0;
        int step = 0;

        unsigned ibo = 0;
        unsigned size = 0;

        std::vector<unsigned> dist, indices;

        vec2 prevPos;
};

#endif // STRUCTS_H
