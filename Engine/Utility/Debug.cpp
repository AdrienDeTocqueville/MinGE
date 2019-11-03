#include "Utility/Debug.h"
#include "Assets/Program.h"
#include "Systems/GraphicEngine.h"

std::vector<vec3> Debug::points;
std::vector<vec3> Debug::pColors;

std::vector<vec3> Debug::lines;
std::vector<vec3> Debug::lColors;

Program* Debug::program = nullptr;

bool Debug::linesDepthTest = true;

/// Methods (public)
void Debug::drawPoint(vec3 _point, vec3 _color)
{
	points.push_back(_point);
	pColors.push_back(_color);
}

void Debug::drawLine(vec3 _from, vec3 _to, vec3 _color)
{
	lines.push_back(_from);
	lColors.push_back(_color);

	lines.push_back(_to);
	lColors.push_back(_color);
}

void Debug::drawVector(vec3 _point, vec3 _vector, vec3 _color)
{
	drawLine(_point, _point + _vector, _color);
}

/// Methods (private)
static unsigned vbo = 0, vao = 0;
void Debug::init()
{
	program = Program::get("debug");

	glCheck(glGenBuffers(1, &vbo));
	glCheck(glGenVertexArrays(1, &vao));
}

void Debug::destroy()
{
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}

void Debug::update()
{
	if (!points.size() && !lines.size())
		return;

	GraphicEngine::get()->setMatrix(GE_MODEL, mat4(1.0f));
	GraphicEngine::get()->computeMVP();

	program->use();
	//program->send(0, GraphicEngine::get()->getMatrix(GE_MVP));

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
	pColors.clear();
	lines.clear();
	lColors.clear();
}

void Debug::drawPoints()
{
	unsigned offset[2];

	offset[0] = points.size() *sizeof(vec3);
	offset[1] = offset[0]+ pColors.size() *sizeof(vec3);

	/// VBO
	GL::BindVertexBuffer(vbo);

		glBufferData(GL_ARRAY_BUFFER, offset[1], nullptr, GL_STATIC_DRAW);

		glBufferSubData(GL_ARRAY_BUFFER, 0, offset[0], &points[0]);
		glBufferSubData(GL_ARRAY_BUFFER, offset[0], offset[1]-offset[0], &pColors[0]);

	/// VAO
	GL::BindVertexArray(vao);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, ((char*)nullptr + (offset[0])));


		glDrawArrays(GL_POINTS, 0, points.size());
}

void Debug::drawLines()
{
	unsigned offset[2];

	offset[0] = lines.size() *sizeof(vec3);
	offset[1] = offset[0]+ lColors.size() *sizeof(vec3);

	/// VBO
	GL::BindVertexBuffer(vbo);

		glCheck(glBufferData(GL_ARRAY_BUFFER, offset[1], nullptr, GL_STATIC_DRAW));

		glCheck(glBufferSubData(GL_ARRAY_BUFFER, 0, offset[0], &lines[0]));
		glCheck(glBufferSubData(GL_ARRAY_BUFFER, offset[0], offset[1]-offset[0], &lColors[0]));

	/// VAO
	GL::BindVertexArray(vao);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, ((char*)nullptr + (offset[0])));


	glDrawArrays(GL_LINES, 0, lines.size());
}
