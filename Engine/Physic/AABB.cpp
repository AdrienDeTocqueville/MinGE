#include "Physic/AABB.h"
#include "Assets/Program.h"
#include "Systems/GraphicEngine.h"

bool AABB::drawAABBs = false;

unsigned AABB::vbo;
unsigned AABB::vao;

std::vector<vec3> AABB::vertices;
std::vector<vec3> AABB::colors;

void AABB::prepare()
{
    if (!drawAABBs)
        return;

    vertices.reserve(vertices.size() + 16);
    colors.reserve(colors.size() + 16);

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

    color = vec3(0.3f, 0.78f, 0.84f);
}

void AABB::draw()
{
    if (!drawAABBs)
        return;

    static Program* p = Program::get("debug.vert", "debug.frag");

    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);

    unsigned offset[2];
    offset[0] = vertices.size() *sizeof(vec3);
    offset[1] = offset[0]+ colors.size() *sizeof(vec3);

    /// VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glBufferData(GL_ARRAY_BUFFER, offset[1], nullptr, GL_STATIC_DRAW);

        glBufferSubData(GL_ARRAY_BUFFER, 0, offset[0], &vertices[0]);
        glBufferSubData(GL_ARRAY_BUFFER, offset[0], offset[1]-offset[0], &colors[0]);

    /// VAO
    glBindVertexArray(vao);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, ((char*)nullptr + (offset[0])));


    GraphicEngine::get()->setMatrix(GE_MODEL, mat4(1.0f));
    GraphicEngine::get()->computeMVP();

    p->use();
    p->send(0, GraphicEngine::get()->getMatrix(GE_MVP));

    glPushAttrib(GL_POLYGON_BIT);

        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_QUADS, 0, vertices.size());

    glPopAttrib();

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    AABB::vertices.clear();
    AABB::colors.clear();
}

bool AABB::collide(AABB* a, AABB* b)
{
    for (unsigned i(0) ; i < 3 ; i++)
    {
        if (a->center[i] + a->dim[i] <= b->center[i] - b->dim[i] || a->center[i] - a->dim[i] >= b->center[i] + b->dim[i])
            return false;
    }

    a->color = b->color = vec3(0.0f, 1.0f, 0.70f);

    return true;
}
