#ifndef TERRAIN_H
#define TERRAIN_H

#include "Assets/Mesh.h"
#include "Util/Structs.h"

class Terrain : public Mesh
{
    public:
        Terrain(std::string _file);
        virtual ~Terrain();

        /// Methods (public)
            void updateTree(vec3 _position);

            void render(Transform* _tr) override;

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

#endif // TERRAIN_H
