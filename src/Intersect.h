#pragma once
#include "Physics/Body.h"
#include "Math/Vector.h"
#include "Physics/ShapeBase.h"

namespace Cacti
{
	bool Intersect(Body* bodyA, Body* bodyB);
}