#include "Utility/Debug.h"
#include "Systems/GraphicEngine.h"

#include "Assets/Program.h"
#include "Assets/Material.h"

std::vector<Debug::Vertex> Debug::points, Debug::lines;

bool Debug::linesDepthTest = true;

/// Methods (public)
void Debug::drawPoint(vec3 _point, vec3 _color)
{
	points.push_back({_point, _color});
}

void Debug::drawLine(vec3 _from, vec3 _to, vec3 _color)
{
	lines.push_back({_from, _color});
	lines.push_back({_to, _color});
}

void Debug::drawVector(vec3 _point, vec3 _vector, vec3 _color)
{
	drawLine(_point, _point + _vector, _color);
}

/// Methods (private)
static MaterialRef material = NULL;
static unsigned vbo = 0, vao = 0;

void Debug::init()
{
	material = Material::create("debug");

	vao = GL::GenVertexArray();
	vbo = GL::GenBuffer();
}

void Debug::destroy()
{
	GL::DeleteVertexArray(vao);
	GL::DeleteBuffer(vbo);

	material = nullptr;
}

void Debug::update()
{
	if (!points.size() && !lines.size())
		return;

	material->bind(RenderPass::Forward);

	glPushAttrib(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	/// Points
	if (points.size())
		drawPoints();

	/// Lines
	if (lines.size())
		drawLines();

	glPopAttrib();

	points.clear();
	lines.clear();
}

void Debug::drawPoints()
{
	GL::BindVertexArray(vao);
	GL::BindVertexBuffer(vbo);

		glCheck(glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(Debug::Vertex), points.data(), GL_STATIC_DRAW));

		glCheck(glEnableVertexAttribArray(0));
		glCheck(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Debug::Vertex), 0));

		glCheck(glEnableVertexAttribArray(1));
		glCheck(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Debug::Vertex), BUFFER_OFFSET(sizeof(vec3))));

	glDrawArrays(GL_POINTS, 0, points.size());
}

void Debug::drawLines()
{
	GL::BindVertexArray(vao);
	GL::BindVertexBuffer(vbo);

		glCheck(glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(Debug::Vertex), lines.data(), GL_STATIC_DRAW));

		glCheck(glEnableVertexAttribArray(0));
		glCheck(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Debug::Vertex), 0));

		glCheck(glEnableVertexAttribArray(1));
		glCheck(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Debug::Vertex), BUFFER_OFFSET(sizeof(vec3))));

	glDrawArrays(GL_LINES, 0, lines.size());
}
