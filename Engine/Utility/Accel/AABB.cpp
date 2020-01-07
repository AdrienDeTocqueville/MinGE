#include "Utility/Accel/AABB.h"

bool AABB::collide(AABB* a, AABB* b)
{
	/*
	for (unsigned i(0) ; i < 3 ; i++)
	{
		if (a->center[i] + a->dim[i] <= b->center[i] - b->dim[i] || a->center[i] - a->dim[i] >= b->center[i] + b->dim[i])
			return false;
	}*/
	for (unsigned i(0) ; i < 3 ; i++)
	{
		if (a->bounds[1][i] <= b->bounds[0][i] || a->bounds[0][i] >= b->bounds[1][i])
			return false;
	}

#ifdef DRAWAABB
	a->color = b->color = vec3(0.0f, 1.0f, 0.70f);
#endif

	return true;
}

void AABB::init(vec3 _min, vec3 _max)
{
	bounds[0] = _min;
	bounds[1] = _max;

#ifdef DRAWAABB
	color = default_color = vec3(0.9f, 0.1f, 0.0f);
#endif
}

void AABB::extend(const AABB& box)
{
	bounds[0] = min(bounds[0], box.bounds[0]);
	bounds[1] = max(bounds[1], box.bounds[1]);
}

float AABB::volume() const
{
	 vec3 d = bounds[1] - bounds[0];
	 return d.x * d.y * d.z;
}

bool AABB::operator==(const AABB& box)
{
    return (bounds[0] == box.bounds[0] &&
			bounds[1] == box.bounds[1]);
}

#ifdef DRAWAABB
#include "Systems/GraphicEngine.h"
#include "Renderer/GLDriver.h"
#include "Assets/Shader.h"
#include "Assets/Shader.inl"

bool AABB::drawAABBs = true;

std::vector<vec3> AABB::vertices;
std::vector<vec3> AABB::colors;

void AABB::prepare(float padding)
{
	if (!drawAABBs)
		return;

	vertices.reserve(vertices.size() + 16);
	colors.reserve(colors.size() + 16);

	vec3 center = 0.5f * (bounds[0] + bounds[1]);
	vec3 dim = bounds[1] - center + vec3(padding);

	vertices.push_back(center + vec3( dim.x, dim.y,  dim.z));
	vertices.push_back(center + vec3(-dim.x, dim.y,  dim.z));
	vertices.push_back(center + vec3(-dim.x, dim.y, -dim.z));
	vertices.push_back(center + vec3( dim.x, dim.y, -dim.z));

	vertices.push_back(center + vec3( dim.x, -dim.y,  dim.z));
	vertices.push_back(center + vec3(-dim.x, -dim.y,  dim.z));
	vertices.push_back(center + vec3(-dim.x, -dim.y, -dim.z));
	vertices.push_back(center + vec3( dim.x, -dim.y, -dim.z));

	vertices.push_back(center + vec3( dim.x,  dim.y,  dim.z));
	vertices.push_back(center + vec3(-dim.x,  dim.y,  dim.z));
	vertices.push_back(center + vec3(-dim.x, -dim.y,  dim.z));
	vertices.push_back(center + vec3( dim.x, -dim.y,  dim.z));

	vertices.push_back(center + vec3( dim.x,  dim.y, -dim.z));
	vertices.push_back(center + vec3(-dim.x,  dim.y, -dim.z));
	vertices.push_back(center + vec3(-dim.x, -dim.y, -dim.z));
	vertices.push_back(center + vec3( dim.x, -dim.y, -dim.z));



	for (unsigned i(0) ; i < 16 ; i++)
		colors.push_back(color);

	color = default_color;
}

void AABB::draw()
{
	if (!drawAABBs)
		return;

	static MaterialRef material = Material::create("debug");

	static unsigned vao = 0, vbo = 0;
	if (vao == 0) vao = GL::GenVertexArray();
	if (vbo == 0) vbo = GL::GenBuffer();

	unsigned offset[2];
	offset[0] = vertices.size() *sizeof(vec3);
	offset[1] = offset[0]+ colors.size() *sizeof(vec3);

	/// VBO
	GL::BindVertexBuffer(vbo);

		glBufferData(GL_ARRAY_BUFFER, offset[1], nullptr, GL_STATIC_DRAW);

		glBufferSubData(GL_ARRAY_BUFFER, 0, offset[0], &vertices[0]);
		glBufferSubData(GL_ARRAY_BUFFER, offset[0], offset[1]-offset[0], &colors[0]);

	/// VAO
	GL::BindVertexArray(vao);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, ((char*)nullptr + (offset[0])));


	Shader::setBuiltin("MATRIX_M", mat4(1.0f));
	material->bind(RenderPass::Forward);

	glPushAttrib(GL_POLYGON_BIT);

		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_QUADS, 0, vertices.size());

	glPopAttrib();

	AABB::vertices.clear();
	AABB::colors.clear();
}

#endif
