#include "Components/Skybox.h"
#include "Components/Transform.h"
#include "Components/Camera.h"

#include "Renderer/RenderContext.inl"
#include "Renderer/CommandKey.h"

#include "Utility/Time.h"


static vec3 ABCDE[] = {
	{ -0.2592f, -0.2608f, -1.4630f },
	{  0.0008f,  0.0092f,  0.4275f },
	{  0.2125f,  0.2102f,  5.3251f },
	{ -0.8989f, -1.6537f, -2.5771f },
	{  0.0452f,  0.0529f,  0.3703f },
};
static vec3 ABCDE_t[] = {
	{ -0.0193f, -0.0167f,  0.1787f },
	{ -0.0665f, -0.0950f, -0.3554f },
	{ -0.0004f, -0.0079f, -0.0227f },
	{ -0.0641f, -0.0441f,  0.1206f },
	{ -0.0033f, -0.0109f, -0.0670f },
};
static void computePerezCoeff(float _turbidity, vec4 _outPerezCoeff[5])
{
	const vec3 turbidity = vec3(_turbidity);
	for (uint32_t i = 0; i < 5; ++i)
	{
		const vec3 tmp = ABCDE_t[i] * turbidity + ABCDE[i];
		_outPerezCoeff[i] = vec4(tmp, 0.0f);
	}
}


typedef std::map<float, vec3> KeyMap;
static KeyMap skyLuminanceXYZTable = {
	{  0.0f, { 0.308f,    0.308f,    0.411f    } },
	{  1.0f, { 0.308f,    0.308f,    0.410f    } },
	{  2.0f, { 0.301f,    0.301f,    0.402f    } },
	{  3.0f, { 0.287f,    0.287f,    0.382f    } },
	{  4.0f, { 0.258f,    0.258f,    0.344f    } },
	{  5.0f, { 0.258f,    0.258f,    0.344f    } },
	{  7.0f, { 0.962851f, 1.000000f, 1.747835f } },
	{  8.0f, { 0.967787f, 1.000000f, 1.776762f } },
	{  9.0f, { 0.970173f, 1.000000f, 1.788413f } },
	{ 10.0f, { 0.971431f, 1.000000f, 1.794102f } },
	{ 11.0f, { 0.972099f, 1.000000f, 1.797096f } },
	{ 12.0f, { 0.972385f, 1.000000f, 1.798389f } },
	{ 13.0f, { 0.972361f, 1.000000f, 1.798278f } },
	{ 14.0f, { 0.972020f, 1.000000f, 1.796740f } },
	{ 15.0f, { 0.971275f, 1.000000f, 1.793407f } },
	{ 16.0f, { 0.969885f, 1.000000f, 1.787078f } },
	{ 17.0f, { 0.967216f, 1.000000f, 1.773758f } },
	{ 18.0f, { 0.961668f, 1.000000f, 1.739891f } },
	{ 20.0f, { 0.264f,    0.264f,    0.352f    } },
	{ 21.0f, { 0.264f,    0.264f,    0.352f    } },
	{ 22.0f, { 0.290f,    0.290f,    0.386f    } },
	{ 23.0f, { 0.303f,    0.303f,    0.404f    } },
};

static vec3 getLuminance(float time, const KeyMap &table)
{
	typename KeyMap::const_iterator itUpper = table.upper_bound(time + 1e-6f);
	typename KeyMap::const_iterator itLower = itUpper;
	--itLower;

	if (itLower == table.end())
		return itUpper->second;

	if (itUpper == table.end())
		return itLower->second;

	float lowerTime = itLower->first;
	const vec3& lowerVal = itLower->second;
	float upperTime = itUpper->first;
	const vec3& upperVal = itUpper->second;

	if (lowerTime == upperTime)
		return lowerVal;

	const float tt = (time - lowerTime) / (upperTime - lowerTime);
	const vec3 result = mix(lowerVal, upperVal, tt);
	return result;
}



Skybox::Skybox():
	Skybox(Material::create("skybox"))
{
	vec4 perezCoeff[5];
	computePerezCoeff(2.15f, perezCoeff);
	float time = 6.118;

	sky->set("perezCoeff", perezCoeff, 5);
	sky->set("skyLuminance", getLuminance(time, skyLuminanceXYZTable));
	sky->set("exposition", 0.1f);
	sky->set("sunSize", 0.02f);
	sky->set("sunBloom", 3.0f);
}

Skybox::Skybox(MaterialRef material):
	sky(material)
{
	mesh = Mesh::createQuad(MeshData::Points, vec2(1.0f), uvec2(8));
}

/// Methods (public)
Skybox* Skybox::clone() const
{
	return new Skybox(sky);
}

void Skybox::render(RenderContext *ctx, uint32_t view_id) const
{
	auto *cmd = ctx->create<DrawElements>();
	cmd->vao = mesh->getVAO();
	memcpy(&(cmd->submesh), mesh->getSubmeshes().data(), sizeof(Submesh));

	uint64_t key = CommandKey::encode(view_id, RenderPass::Skybox, sky->getId(), 0.0f);
	ctx->add(key, cmd);
}
