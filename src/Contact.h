#pragma once
#include "Math/Vector.h"
#include "Physics/Body.h"
namespace Cacti
{
	struct Contact
	{
		Vec3 ptOnA_WorldSpace;
		Vec3 ptOnB_WorldSpace;
		Vec3 ptOnA_LocalSpace;
		Vec3 ptOnB_LocalSpace;

		Vec3 normal;	// In World Space coordinates
		float separationDistance;	// positive when non-penetrating, negative when penetrating
		float timeOfImpact;

		Body* bodyA;
		Body* bodyB;
	};

	void ResolveContact(Contact& contact);
}