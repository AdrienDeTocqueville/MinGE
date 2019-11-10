#include "Components/Skybox.h"
#include "Components/Transform.h"
#include "Components/Camera.h"

#include "Renderer/CommandKey.h"

Skybox::Skybox(vec3 c0, vec3 c1):
	Skybox(Material::create("skybox"))
{
	sky->set("c0", c0);
	sky->set("c1", c1);
}

Skybox::Skybox(MaterialRef material):
	sky(material)
{
	cube = Mesh::createCube();
}

/// Methods (public)
Skybox* Skybox::clone() const
{
	return new Skybox(sky);
}

void Skybox::render(CommandBucket *bucket, uint32_t view_id) const
{
	uint64_t key = CommandKey::encode(view_id, RenderPass::Skybox, sky->getId(), 0.0f);
	DrawElements *cmd = bucket->add<DrawElements>(key);
	cmd->vao = cube->getVAO();
	memcpy(&(cmd->submesh), cube->getSubmeshes().data(), sizeof(Submesh));
}
