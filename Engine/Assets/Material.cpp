#include "Assets/Material.h"
#include "Utility/Error.h"

#include "Systems/GraphicEngine.h"
#include "Components/Light.h"
#include "Components/Camera.h"

enum ShaderBuiltins
{
	// Camera
	MATRIX_VP, CLIP_PLANE, CAMERA_POS,
	// Light
	LIGHT_POS, LIGHT_DIFF, LIGHT_AMBIENT, LIGHT_CONST, LIGHT_LINEAR, LIGHT_QUADRATIC,
	// Model
	MATRIX_M, MATRIX_N,
};

static const std::vector<std::string> shader_builtins = {
	// Camera
	"MATRIX_VP", "clipPlane", "cameraPosition",
	// Light
	"lightPosition", "diffuseColor", "ambientCoefficient", "aConstant", "aLinear", "aQuadratic",
	// Model
	"MATRIX_M", "MATRIX_N",
};

static const std::map<GLuint, uint32_t> uniform_type_size = {
	{GL_FLOAT,	sizeof(float)},
	{GL_FLOAT_VEC2, sizeof(vec2)},
	{GL_FLOAT_VEC3, sizeof(vec3)},
	{GL_FLOAT_VEC4, sizeof(vec4)},
	{GL_FLOAT_MAT3, sizeof(mat3)},
	{GL_FLOAT_MAT4, sizeof(mat4)},
	{GL_SAMPLER_2D, sizeof(Texture*)},
};

std::weak_ptr<Material> Material::basic;

Material::Material(Program *_program):
	program(_program)
{
	for (auto& u : program->getUniforms())
	{
		if (uniform_type_size.find(u.type) == uniform_type_size.end())
			Error::add(USER_ERROR, "Unsupported uniform type");


		auto it = std::find(shader_builtins.begin(), shader_builtins.end(), u.name);
		if (it != shader_builtins.end())
			builtin_props.push_back({u.type, u.location, (size_t)(it - shader_builtins.begin())});
		else
		{
			property_names[u.name] = properties.size();
			properties.push_back({u.type, u.location, uniforms.size()});
			uniforms.resize(uniforms.size() + uniform_type_size.at(u.type));
		}
	}
}

MaterialRef Material::clone() const
{
	return std::shared_ptr<Material>(new Material(program));
}

void Material::bind() const
{
	GL::UseProgram(program->program);

	Camera *camera = Camera::current;
	Light *light = GraphicEngine::get()->getLight();
	for (const Property& prop : builtin_props)
	{
		switch (prop.offset)
		{
		case MATRIX_VP:
			set_uniform(prop.location, GraphicEngine::get()->getMatrix(GE_VP)); break;
		case CLIP_PLANE:
			set_uniform(prop.location, camera->getClipPlane()); break;
		case CAMERA_POS:
			set_uniform(prop.location, camera->getPosition()); break;

		case LIGHT_POS:
			set_uniform(prop.location, light->getPosition()); break;
		case LIGHT_DIFF:
			set_uniform(prop.location, light->getDiffuseColor()); break;
		case LIGHT_AMBIENT:
			set_uniform(prop.location, light->getAmbientCoefficient()); break;
		case LIGHT_CONST:
			set_uniform(prop.location, light->getAttenuation().x); break;
		case LIGHT_LINEAR:
			set_uniform(prop.location, light->getAttenuation().y); break;
		case LIGHT_QUADRATIC:
			set_uniform(prop.location, light->getAttenuation().z); break;

		case MATRIX_M:
			set_uniform(prop.location, GraphicEngine::get()->getMatrix(GE_MODEL)); break;
		case MATRIX_N:
			set_uniform(prop.location, GraphicEngine::get()->getMatrix(GE_MODEL)); break;
		}
	}

	for (const Property& prop : properties)
	{
		const void *value = uniforms.data() + prop.offset;
		switch (prop.type)
		{
		case GL_SAMPLER_2D:
		{
			Texture *t = *(Texture**)value;
			t->use(0);
			glCheck(glUniform1i(prop.location, 0));
			break;
		}
		case GL_FLOAT:
			glCheck(glUniform1f(prop.location, *(float*)value));
			break;
		case GL_FLOAT_VEC2:
			glCheck(glUniform2fv(prop.location, 1, (float*)value));
			break;
		case GL_FLOAT_VEC3:
			glCheck(glUniform3fv(prop.location, 1, (float*)value));
			break;
		case GL_FLOAT_VEC4:
			glCheck(glUniform4fv(prop.location, 1, (float*)value));
			break;
		case GL_FLOAT_MAT3:
			glCheck(glUniformMatrix3fv(prop.location, 1, GL_FALSE, (float*)value));
			break;
		case GL_FLOAT_MAT4:
			glCheck(glUniformMatrix4fv(prop.location, 1, GL_FALSE, (float*)value));
			break;
		}
	}
}

MaterialRef Material::create(std::string name)
{
	return std::shared_ptr<Material>(new Material(Program::get(name)));
}

MaterialRef Material::getDefault()
{
	if (auto shared = basic.lock())
		return shared;
	auto shared = std::shared_ptr<Material>(new Material(Program::getDefault()));
	basic = shared;

	return shared;
}

template<>
void Material::set(uint32_t prop, Texture *value)
{
	*(Texture**)(uniforms.data() + properties[prop].offset) = value;
}
