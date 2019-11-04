#include "Components/Skybox.h"
#include "Components/Transform.h"
#include "Components/Camera.h"

Skybox::Skybox()
{
	//sky = Mesh::createCube(new SkyboxMaterial("leCiel"));
}

/// Methods (public)
Skybox* Skybox::clone() const
{
	return new Skybox();
}

void Skybox::render() const
{
	glPushAttrib(GL_POLYGON_BIT);
	glPushAttrib(GL_DEPTH_BUFFER_BIT);

		glCullFace(GL_FRONT);
		glDisable(GL_DEPTH_TEST);

		//sky->render(tr, sky->getMaterials());

	glPopAttrib();
	glPopAttrib();
}
