#ifndef ANIMATOR_H
#define ANIMATOR_H

#include "Components/Component.h"
#include "Meshes/AnimatedModel.h"

class Animator : public Component
{
    friend class Graphic;

    public:
        Animator();
        virtual ~Animator();

        /// Methods (public)
            virtual Animator* clone() const override;

            void loadAnimation(int _index, bool _repeat = false);

            void update();

        /// Getter
            Transform* getBone(unsigned _index);
            Transform* getBone(std::string _name);

    private:
        /// Methods (private)
            virtual void onRegister() override;

        /// Setter (private)
            void setGraphic(Graphic* _graphic);

        /// Attributes (private)
            AnimatedModel* model;

            AnimatedModel::Animation* current;
            float accumulator;
            bool loop;

            std::vector<unsigned> trKeys;
            std::vector<unsigned> roKeys;

            std::vector<Transform*> bones;
            std::vector<mat4> matrices;

            int root;
};

#endif // ANIMATOR_H
