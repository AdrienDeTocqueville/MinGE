#include "../PlayerScript.h"
#include "../CameraScript.h"

void test_ui()
{
	// Cube
	Entity::create("Cube")
		->insert<Graphic>(Mesh::createCube());
	Material::getDefault()->define("MAIN_LIGHT"); // disable shadows

	// UI
	//Entity::create("UI") ->insert<UIView>(vec4(0.5f, 0.5f, 0.5f, 0.5f));

	// Camera
	Entity::create("MainCamera")
		->insert<Camera>(70, 0.1f, 1000.0f, vec3(0.67f, 0.92f, 1.0f))
		->insert<Skybox>()
		->insert<CameraScript>(nullptr, 0.2f, 7.0f);
}
