#ifndef TERRAINSCRIPT_H
#define TERRAINSCRIPT_H

#include <MinGE.h>

class TerrainScript : public Script
{
    public:
        TerrainScript(std::string _targetTag):
            targetTag(_targetTag)
        {
        }

        /// Methods (public)
            void start() override
            {
                terrain = reinterpret_cast<Terrain*>(getComponent<Graphic>()->getMesh()) ;
                target = Entity::findByTag(targetTag)->getTransform();
            }

            void update() override
            {
                vec3 pos = tr->getToLocalSpace(target->position);
                terrain->updateTree(pos);
            }


    private:
        Terrain* terrain;
        Transform* target;

        std::string targetTag;
};

#endif // TERRAINSCRIPT_H
