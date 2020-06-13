#include <mutex>

#include "Profiler/profiler.h"

#include "Render/Debug.h"
#include "Render/GLDriver.h"
#include "Render/Shaders/Material.h"
#include "Render/Shaders/Shader.h"

#include "Structures/Bounds.h"

struct Vertex
{
	vec3 pos, color;
};
static std::mutex points_lock, lines_lock;
static std::vector<Vertex> points, lines;


void Debug::point(vec3 _point, vec3 color)
{
	const std::lock_guard<std::mutex> lock(points_lock);
	points.push_back({_point, color});
}

void Debug::line(vec3 _from, vec3 _to, vec3 color)
{
	const std::lock_guard<std::mutex> lock(lines_lock);
	lines.push_back({_from, color});
	lines.push_back({_to, color});
}

void Debug::vector(vec3 _point, vec3 _vector, vec3 color)
{
	line(_point, _point + _vector, color);
}

void Debug::aabb(const AABB &box, vec3 color)
{
	OBB o;
	o.init(box);
	obb(o, color);
}

void Debug::obb(const OBB &box, vec3 color)
{
	vec3 points[4] = {
		box.base,
		box.base + box.axis[1],
		box.base + box.axis[1] + box.axis[2],
		box.base + box.axis[2],
	};

	for (int p = 0; p < 4; p++)
	{
		line(points[p], points[(p + 1) % 4], color);
		line(points[p]+box.axis[0], points[(p + 1) % 4]+box.axis[0], color);
		line(points[p], points[p]+box.axis[0], color);
	}
}

void Debug::sphere(const struct Sphere &sphere, vec3 color)
{
	point(sphere.center, color);

	float radius(sqrt(sphere.radius2));
	unsigned slices = 20, stacks = 10;

	const float iStacks = 1.0f / (float)(stacks - 1);
	const float iSlices = 1.0f / (float)(slices - 1);

	float slice_step = 2 * PI * iSlices;
	float stack_step = PI * iStacks;
	float slice_angle, stack_angle;

	// Generate vertices
	for (unsigned i = 0; i <= stacks; ++i)
	{
		stack_angle = 0.5f * PI - i * stack_step;
		float xy = cosf(stack_angle);
		float z = sinf(stack_angle);

		for (unsigned j = 0; j <= slices; ++j)
		{
			slice_angle = j * slice_step;

			vec3 normal(xy * cosf(slice_angle), xy * sinf(slice_angle), z);

			point(sphere.center + normal * radius, color);
		}
	}
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
	material = Material::create(Shader::load("asset:shader/debug"));

	vao = GL::GenVertexArray();
	vbo = GL::GenBuffer();
}

void Debug::destroy()
{
	GL::DeleteBuffer(vbo);
	GL::DeleteVertexArray(vao);

	material.destroy();
}

void flush_points()
{
	MICROPROFILE_SCOPEGPUI("debug_points", -1);

	GL::BindVertexArray(vao);
	GL::BindVertexBuffer(vbo);

		glCheck(glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(Vertex), points.data(), GL_STATIC_DRAW));

		glCheck(glEnableVertexAttribArray(0));
		glCheck(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0));

		glCheck(glEnableVertexAttribArray(1));
		glCheck(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)sizeof(vec3)));

	glDrawArrays(GL_POINTS, 0, (GLsizei)points.size());
	points.clear();
}

void flush_lines()
{
	MICROPROFILE_SCOPEGPUI("debug_lines", -1);

	GL::BindVertexArray(vao);
	GL::BindVertexBuffer(vbo);

		glCheck(glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(Vertex), lines.data(), GL_STATIC_DRAW));

		glCheck(glEnableVertexAttribArray(0));
		glCheck(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0));

		glCheck(glEnableVertexAttribArray(1));
		glCheck(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)sizeof(vec3)));

	glDrawArrays(GL_LINES, 0, (GLsizei)lines.size());
	lines.clear();
}

void Debug::flush()
{
	if (!points.size() && !lines.size())
		return;

	MICROPROFILE_SCOPEI("RENDER_ENGINE", "debug_draw");
	MICROPROFILE_SCOPEGPUI("debug_draw", -1);

	Material::materials.get<0>(material.id())->bind(RenderPass::Forward);

	//GL::Disable(GL::DepthTest);

	if (points.size())	flush_points();
	if (lines.size())	flush_lines();
}
