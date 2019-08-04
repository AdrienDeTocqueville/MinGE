#ifndef ANIMATEDMODEL_H
#define ANIMATEDMODEL_H

#include "Assets/Mesh.h"

class AnimatedModel : public Mesh
{
    friend class Animator;

    // Structure definitions
    struct Bone
    {
        Bone(std::string _name, vec3 _position, float _angle, vec3 _axis):
            position(_position), rotation(angleAxis(_angle, _axis)),
            name(_name),
            parent(-1)
        { }

        vec3 position;
        quat rotation;

        mat4 inverseBindMatrix;

        std::string name;

        int parent;
        std::vector<int> children;
    };

    struct Track
    {
        Track(unsigned _boneId):
            boneId(_boneId)
        { }

        unsigned boneId;
        std::vector<std::pair<float, vec3>> translations;
        std::vector<std::pair<float, quat>> rotations;
    };

    struct Animation
    {
        Animation(std::string _name, float _duration):
            name(_name), duration(_duration)
        { }

        std::string name;
        float duration;

        std::vector<Track> tracks;
    };
    // Structure definitions

    public:
        AnimatedModel(std::string _mesh);
        virtual ~AnimatedModel();

        /// Methods (public)
            virtual void render(Transform* _tr, const std::vector<Material*>& _materials) override;

        /// Getters
            unsigned getBoneIndex(std::string _name);
            unsigned getAnimationIndex(std::string _name);

    protected:
        /// Methods (protected)
            virtual void loadBuffers() override;

            bool load_mesh_xml();
            bool load_skeleton_xml();

            bool load_material();

            void reduceKeyframes(Track& _track);

        /// Attributes
            std::vector<ivec4> boneIds;
            std::vector< vec4> weights;

            std::vector<Bone> bones;
            Bone* root;

            std::vector<Animation> animations;

            unsigned ibo = 0;
            std::vector<unsigned short> indices;

            std::string mesh, skeleton, material;

    private:
        void computeMatrix(Bone* _bone);
};

#endif // ANIMATEDMODEL_H
