#ifndef ANIMATEDMODEL_H
#define ANIMATEDMODEL_H

#include "Assets/Mesh.h"

class AnimatedModel : public Mesh
{
    // Structure definitions
    struct Bone
    {
        Bone(unsigned _id, std::string _name, vec3 _position, float _angle, vec3 _axis):
            id(_id), name(_name), position(_position), rotation(angleAxis(_angle, _axis)),
            parent(-1)
        {
            initialP = position;
            initialR = rotation;
        }

        unsigned id;
        std::string name;

        vec3 position, initialP;
        quat rotation, initialR;

        mat4 matrix, inverseBindMatrix;
        mat4 pose;

        int parent;
        std::vector<int> children;
    };

    struct Keyframe
    {
        Keyframe(float _time, vec3 _translate, float _angle, vec3 _axis):
            time(_time), translate(_translate), rotate(angleAxis(_angle, _axis))
        { }

        float time;

        vec3 translate;
        quat rotate;
    };

    struct Track
    {
        Track(unsigned _boneId):
            boneId(_boneId)
        { }

        unsigned boneId;
        std::vector<Keyframe> keyframes;
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
            void updateSkeleton();

            virtual void render(Transform* _tr);

            void loadAnimation(int _index, bool _repeat = false);

    protected:
        /// Methods (protected)
            virtual void loadBuffers();

            bool load_mesh_xml();
            bool load_skeleton_xml();

            bool load_material();

        /// Attributes
            std::vector<ivec4> boneIds;
            std::vector< vec4> weights;

            std::vector<Bone> bones;
            Bone* root;

            std::vector<Animation> animations;
            Animation* current;
            float accumulator;
            bool loop;

            unsigned ibo = 0;
            std::vector<unsigned short> indices;

            std::string mesh, skeleton, material;

    private:
        unsigned getBoneIndex(std::string _name);
        unsigned getAnimationIndex(std::string _name);

        void computeMatrix(Bone* _bone);
};

#endif // ANIMATEDMODEL_H
