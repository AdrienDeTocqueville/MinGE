#ifndef SKYBOX_H
#define SKYBOX_H

#include "Components/Component.h"
#include "Assets/Mesh.h"

class Skybox : public Component
{
    public:
        Skybox();
        virtual ~Skybox();

        /// Methods (public)
            virtual Skybox* clone() const override;

            void render();

    private:
        Mesh* sky;
};

#endif // SKYBOX_H
