#ifndef MODEL_H
#define MODEL_H

#include "Assets/Mesh.h"

class Model : public Mesh
{
	public:
		Model();
		Model(std::string _file);
		virtual ~Model();

	protected:
		/// Methods (protected)
			bool load_obj();

			bool load_mtl(std::string _path);

		/// Attributes
			std::string file;
};

#endif // MODEL_H
