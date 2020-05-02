#include "Graphics/Debug.h"
#include "Graphics/GLDriver.h"
#include "Graphics/Shaders/Material.h"

#include "Structures/Bounds.h"

struct Vertex
{
	vec3 pos, color;
};
static std::vector<Vertex> points, lines;


void Debug::point(vec3 _point, vec3 _color)
{
	points.push_back({_point, _color});
}

void Debug::line(vec3 _from, vec3 _to, vec3 _color)
{
	lines.push_back({_from, _color});
	lines.push_back({_to, _color});
}

void Debug::vector(vec3 _point, vec3 _vector, vec3 _color)
{
	line(_point, _point + _vector, _color);
}

void Debug::aabb(const AABB &box, vec3 color)
{
	vec3 a = box.bounds[0], b = box.bounds[1];
	vec3 points[2][4] = {
	{
		a,
		vec3(b.x,a.y,a.z),
		vec3(b.x,b.y,a.z),
		vec3(a.x,b.y,a.z),
	},{
		b,
		vec3(a.x,b.y,b.z),
		vec3(a.x,a.y,b.z),
		vec3(b.x,a.y,b.z),
	}
	};

	for (int f = 0; f < 2; f++)
	{
		line(points[f][0], points[f][1], color);
		line(points[f][1], points[f][2], color);
		line(points[f][2], points[f][3], color);
		line(points[f][3], points[f][0], color);
	}
	for (int p = 0; p < 4; p++)
		line(points[0][p], points[1][(p+2)%4], color);
}

inline vec3 intersect_planes(vec4 a, vec4 b, vec4 c)
{
	mat3 A = transpose(mat3(vec3(a), vec3(b), vec3(c)));
	vec3 B = -vec3(a.w, b.w, c.w);
	return inverse(A) * B;
}

void Debug::frustum(const Frustum &f, vec3 color)
{
	vec3 points[2][4];
	int face = Frustum::Near;
	for (int i = 0; i < 2; i++)
	{
		points[i][0] = intersect_planes(f.planes[face], f.planes[Frustum::Top], f.planes[Frustum::Left]);
		points[i][1] = intersect_planes(f.planes[face], f.planes[Frustum::Top], f.planes[Frustum::Right]);
		points[i][2] = intersect_planes(f.planes[face], f.planes[Frustum::Bottom], f.planes[Frustum::Right]);
		points[i][3] = intersect_planes(f.planes[face], f.planes[Frustum::Bottom], f.planes[Frustum::Left]);
		face = Frustum::Far;
	}

	for (int p = 0; p < 4; p++)
	{
		line(points[0][p], points[0][(p + 1) % 4], color);
		line(points[1][p], points[1][(p + 1) % 4], color);
		line(points[0][p], points[1][p], color);
	}
}

static Material material;
static unsigned vbo = 0, vao = 0;

void Debug::init()
{
	material = Material::create(Shader::debug());

	vao = GL::GenVertexArray();
	vbo = GL::GenBuffer();
}

void Debug::destroy()
{
	GL::DeleteBuffer(vbo);
	GL::DeleteVertexArray(vao);

	// TODO: destroy material
}

void flush_points()
{
	GL::BindVertexArray(vao);
	GL::BindVertexBuffer(vbo);

		glCheck(glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(Vertex), points.data(), GL_STATIC_DRAW));

		glCheck(glEnableVertexAttribArray(0));
		glCheck(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0));

		glCheck(glEnableVertexAttribArray(1));
		glCheck(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)sizeof(vec3)));

	glDrawArrays(GL_POINTS, 0, points.size());
	points.clear();
}

void flush_lines()
{
	GL::BindVertexArray(vao);
	GL::BindVertexBuffer(vbo);

		glCheck(glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(Vertex), lines.data(), GL_STATIC_DRAW));

		glCheck(glEnableVertexAttribArray(0));
		glCheck(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0));

		glCheck(glEnableVertexAttribArray(1));
		glCheck(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)sizeof(vec3)));

	glDrawArrays(GL_LINES, 0, lines.size());
	lines.clear();
}

void Debug::flush()
{
	if (!points.size() && !lines.size())
		return;

	Material::materials.get<0>(material.id())->bind(RenderPass::Forward);

	glPushAttrib(GL_DEPTH_BUFFER_BIT);
	{
		glDisable(GL_DEPTH_TEST);

		if (points.size())	flush_points();
		if (lines.size())	flush_lines();

	}
	glPopAttrib();
}
