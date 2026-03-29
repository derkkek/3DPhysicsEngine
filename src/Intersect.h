#pragma once
#include "Physics/Body.h"
#include "Math/Vector.h"
#include "Physics/ShapeBase.h"
#include "Contact.h"
namespace Cacti
{
	bool Intersect(Body* bodyA, Body* bodyB, const float dt,Contact& contact);
	bool RaySphere(const Vec3& rayStart, const Vec3& rayDir, const Vec3& sphereCenter, const float& sphereRadius, float& t1, float& t2);
	bool SphereSphereDynamic(const Sphere* shapeA, const Sphere* shapeB, const Vec3& posA, const Vec3& posB, const Vec3& velA, const Vec3& velB, const float dt, Vec3& ptOnA, Vec3& ptOnB, float& toi);

}