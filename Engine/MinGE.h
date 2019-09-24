/**  Created by Adrien de Tocqueville  **/
/**	  Started in January 2015	   **/

/* ******  TODO  ****** *\
*  Shadows               *
*  Particle effects      *
*  Skeletal animations   *
*  (Billboards)	         *
\* ******************** */

#pragma once


#include <memory>

#include "includes.h"

#include "Engine.h"
#include "Entity.h"

#include "Assets/Material.h"
#include "Assets/Program.h"
#include "Assets/Texture.h"

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

#include "Meshes/AnimatedModel.h"
#include "Meshes/Terrain.h"
#include "Meshes/Model.h"

#include "Physic/DistanceConstraint.h"
#include "Physic/ContactConstraint.h"
#include "Physic/Constraint.h"

#include "Utility/Random.h"
#include "Utility/Debug.h"
#include "Utility/Time.h"
#include "Utility/Tag.h"
#include "Utility/IO/Input.h"
#include "Utility/IO/IOFile.h"
