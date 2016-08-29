#include "Components/Skybox.h"
#include "Components/Transform.h"
#include "Components/Camera.h"

Skybox::Skybox()
{
    sky = Mesh::createCube(new SkyboxMaterial("leCiel"));
}

Skybox::~Skybox()
{
    delete sky;
}

/// Methods (public)
Component* Skybox::clone()
{
    return new Skybox();
}

void Skybox::render()
{
    Transform tr(Camera::current->getTransform()->position);

    glPushAttrib(GL_POLYGON_BIT);
    glPushAttrib(GL_DEPTH_BUFFER_BIT);

        glCullFace(GL_FRONT);
        glDisable(GL_DEPTH_TEST);

        sky->render(&tr);

    glPopAttrib();
    glPopAttrib();
}
