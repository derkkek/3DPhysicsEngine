#include "Intersect.h"
#include <iostream>

namespace Cacti
{
	bool Intersect(Body* bodyA, Body* bodyB, Contact& contact)
	{
		const Sphere* sphereA = (const Sphere*)bodyA->shape;
		const Sphere* sphereB = (const Sphere*)bodyB->shape;

		Vec3 ab = bodyB->position - bodyA->position;

		const float radiusAB = sphereA->radius + sphereB->radius;
		const float lengthSquarre = ab.GetLengthSqr();

		if (lengthSquarre <= radiusAB * radiusAB)
		{
			contact.bodyA = bodyA;
			contact.bodyB = bodyB;
			contact.normal = ab.Normalize();
			contact.ptOnA_WorldSpace = bodyA->position + contact.normal * sphereA->radius;
			contact.ptOnB_WorldSpace = bodyB->position - contact.normal * sphereB->radius;
			return true;
		}

		return false;
	}
}