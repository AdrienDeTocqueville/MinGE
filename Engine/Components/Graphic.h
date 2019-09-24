#ifndef GRAPHIC_H
#define GRAPHIC_H

#include "Components/Component.h"
#include "Assets/Mesh.h"

class Graphic : public Component
{
	friend class Entity;

	public:
		Graphic(Mesh* _mesh);
		virtual ~Graphic();

		/// Methods (public)
			virtual Graphic* clone() const override;

			void render();

		/// Setters
			void setMesh(Mesh* _mesh);

		/// Getters
			Mesh* getMesh() const;
			AABB getAABB() const;

			Material* getMaterial(unsigned _index) const;
			const std::vector<Material*>& getMaterials() const;

	private:
		/// Methods (private)
			virtual void onRegister() override;
			virtual void onDeregister() override;

		/// Attributes
			Mesh* mesh;

			std::vector<Material*> materials;
};

#endif // GRAPHIC_H
