/**  Created by Adrien de Tocqueville  **/

/* ******  TODO  ****** *\
*  Shadows               *
*  Particle effects      *
*  Skeletal animations   *
*  Occlusion culling	 *
*  Optimize materials	 *
*  (Billboards)	         *
\* ******************** */

#pragma once


#include <memory>

#include "Engine.h"
#include "Entity.h"

#include "Systems/GraphicEngine.h"
#include "Systems/PhysicEngine.h"
#include "Systems/ScriptEngine.h"

#include "Assets/PhysicMaterial.h"
#include "Assets/Animation.h"
#include "Assets/Material.h"
#include "Assets/Texture.h"
#include "Assets/Shader.h"
#include "Assets/Scene.h"
#include "Assets/Mesh.h"

#include "Components/Transform.h"
#include "Components/RigidBody.h"
#include "Components/Animator.h"
#include "Components/Graphic.h"
#include "Components/Camera.h"
#include "Components/Skybox.h"
#include "Components/Script.h"
#include "Components/Light.h"

#include "Components/Cylinder.h"
#include "Components/Sphere.h"
#include "Components/Cone.h"
#include "Components/Box.h"

#include "Physic/DistanceConstraint.h"
#include "Physic/ContactConstraint.h"
#include "Physic/Constraint.h"

#include "Utility/helpers.h"
#include "Utility/Random.h"
#include "Utility/Error.h"
#include "Utility/Debug.h"
#include "Utility/Time.h"
#include "Utility/Tag.h"
#include "Utility/IO/Input.h"
