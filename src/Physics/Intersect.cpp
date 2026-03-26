#include "Intersect.h"
#include <iostream>

namespace Cacti
{
	bool Intersect(Body* bodyA, Body* bodyB)
	{
		const Sphere* sphereA = (const Sphere*)bodyA->shape;
		const Sphere* sphereB = (const Sphere*)bodyB->shape;

		const Vec3 ab = bodyB->position - bodyA->position;

		const float radiusAB = sphereA->radius + sphereB->radius;
		const float lengthSquarre = ab.GetLengthSqr();

		if (lengthSquarre <= radiusAB * radiusAB)
		{
			std::cout << "collision" << "\n";
			return true;
		}

		return false;
	}
}