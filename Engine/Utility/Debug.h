#pragma once

#include "Utility/helpers.h"
#include "Assets/Material.h"

class Debug
{
	friend class Engine;
	friend class GraphicEngine;

	public:
		/// Methods (public)
			static void drawPoint(vec3 _point, vec3 _color = vec3(1.0f, 0.1f, 0.1f));

			static void drawLine(vec3 _from, vec3 _to, vec3 _color = vec3(1.0f));
			static void drawVector(vec3 _point, vec3 _vector, vec3 _color = vec3(1.0f));

	private:
		/// Methods (private)
			static void init();
			static void update();
			static void destroy();

			static void drawLines();
			static void drawPoints();

		/// Attributes
			static std::vector<vec3> points;
			static std::vector<vec3> pColors;

			static std::vector<vec3> lines;
			static std::vector<vec3> lColors;

			static MaterialRef material;

			static bool linesDepthTest;
};
