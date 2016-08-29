#ifndef GRAPHIC_H
#define GRAPHIC_H

#include "Components/Component.h"
#include "Assets/Mesh.h"

class Graphic : public Component
{
    public:
        Graphic(Mesh* _mesh);
        virtual ~Graphic();

        /// Methods (public)
            virtual Component* clone() override;

            virtual void registerComponent() override;
            virtual void deregisterComponent() override;

            void render();

        /// Setters
            void setMesh(Mesh* _mesh);

        /// Getters
            Mesh* getMesh() const;

    private:
        /// Attributes
            Mesh* mesh;
};

#endif // GRAPHIC_H
