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
            virtual Graphic* clone() const override;

            virtual void attach(Entity* _entity) override;
            virtual void detach() override;

            virtual void registerComponent() override;
            virtual void deregisterComponent() override;

            void render();

        /// Setters
            void setMesh(Mesh* _mesh);

        /// Getters
            Mesh* getMesh() const;

            Material* getMaterial(unsigned _index) const;
            const std::vector<Material*>& getMaterials() const;

    private:
        /// Attributes
            Mesh* mesh;

            std::vector<Material*> materials;
};

#endif // GRAPHIC_H
