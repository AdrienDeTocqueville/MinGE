#include "Components/Graphic.h"
#include "Components/Animator.h"
#include "Components/Transform.h"

#include "Utility/Time.h"

Animator::Animator():
    model(nullptr), current(nullptr)
{ }

Animator::~Animator()
{
//    for (Transform* bone: bones)
//    {
//        if (bone->getEntity() == nullptr)
//            bone->destroy();
//    }

    bones.clear();
}

/// Methods (public)
Animator* Animator::clone() const
{
    return new Animator();
}

void Animator::loadAnimation(int _index, bool _repeat)
{
    if (_index == -1 || model == nullptr)
    {
        current = nullptr;
        return;
    }

    current = &(model->animations[_index]);
    accumulator = 0.0f;
    loop = _repeat;

    trKeys.clear(); roKeys.clear();
    trKeys.resize(current->tracks.size(), 0);
    roKeys.resize(current->tracks.size(), 0);
}

void Animator::update()
{
    if (current == nullptr || model == nullptr)
        return;

    accumulator += Time::deltaTime;

    if (accumulator >= current->duration)
    {
        if (loop)
        {
            trKeys.clear(); roKeys.clear();
            trKeys.resize(current->tracks.size(), 0);
            roKeys.resize(current->tracks.size(), 0);

            accumulator -= current->duration;
        }
        else
        {
            current = nullptr;
            return;
        }
    }

    for (unsigned t(0) ; t < current->tracks.size() ; t++)
    {
        AnimatedModel::Track& track = current->tracks[t];
        unsigned id = track.boneId;

        // Translations
        auto& trs = track.translations;

        for (; trKeys[t] < trs.size() ; trKeys[t]++)
            if (trs[trKeys[t]].first >= accumulator)
                break;

        float coefT = (accumulator-trs[trKeys[t]-1].first) / (trs[trKeys[t]].first-trs[trKeys[t]-1].first);
        bones[id]->position = model->bones[id].position + mix(trs[trKeys[t]-1].second, trs[trKeys[t]].second, coefT);

        // Rotations
        auto& ros = track.rotations;

        for (; roKeys[t] < ros.size() ; roKeys[t]++)
            if (ros[roKeys[t]].first >= accumulator)
                break;

        float coefR = (accumulator-ros[roKeys[t]-1].first) / (ros[roKeys[t]].first-ros[roKeys[t]-1].first);
        bones[id]->rotation = model->bones[id].rotation * mix(ros[roKeys[t]-1].second, ros[roKeys[t]].second, coefR);
    }

    tr->toMatrix();


    for (unsigned i(0) ; i < bones.size() ; i++)
        matrices[i] = bones[i]->toWorldSpace * model->bones[i].inverseBindMatrix;

//    for (Transform* bone: bones)
//        Debug::drawPoint( bone->getToWorldSpace(vec3(0.0f)), vec3(0, 1, 0));
}

/// Getter
Transform* Animator::getBone(unsigned _index)
{
    return bones[_index];
}

Transform* Animator::getBone(std::string _name)
{
    return bones[model->getBoneIndex(_name)];
}

/// Private
void Animator::onRegister()
{
    setGraphic(find<Graphic>());
}

void Animator::setGraphic(Graphic* _graphic)
{
    if (_graphic == nullptr || typeid(AnimatedModel) != typeid(*_graphic->getMesh()))
        model = nullptr;
    else
        model = static_cast<AnimatedModel*>(_graphic->getMesh());

    if (model == nullptr)
    {
//        for (Transform* bone: bones)
//            if (bone->getEntity() == nullptr)
//                bone->destroy();

        bones.clear();

        if (_graphic != nullptr)
        for (Material* material: _graphic->getMaterials())
        {
            AnimatedModelMaterial* m = static_cast<AnimatedModelMaterial*>(material);

            m->matrices = nullptr;
        }
        return;
    }

    for (Material* material: _graphic->getMaterials())
    {
        AnimatedModelMaterial* m = static_cast<AnimatedModelMaterial*>(material);

        m->matrices = &matrices;
    }


//    bones.reserve(model->bones.size());
//    for (const AnimatedModel::Bone& bone: model->bones)
//        bones.push_back(new Transform(bone.position, bone.rotation, vec3(1.0f)));

    for (unsigned i(0) ; i < model->bones.size() ; i++)
    {
        if (model->bones[i].parent == -1)
            bones[i]->setParent(tr);
        else
            bones[i]->setParent(bones[model->bones[i].parent]);

        for (int child: model->bones[i].children)
            bones[child]->setParent(bones[i]);
    }

    matrices.resize(bones.size());
}
