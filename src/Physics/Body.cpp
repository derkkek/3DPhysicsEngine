#include "Body.h"

namespace Cacti
{
	Body::Body()
		:position(0,0,0), linearVelocity(0,0,0)
	{
	}
	Vec3 Body::GetCenterOfMassWorldSpace() const
	{
		const Vec3 centerOfMass = shape->GetCenterOfMass();
		const Vec3 pos = position + orientation.RotatePoint(centerOfMass);
		return pos;
	}
	Vec3 Body::GetCenterOfMassModelSpace() const
	{
		const Vec3 centerOfMass = shape->GetCenterOfMass();
		return centerOfMass;
	}
	Vec3 Body::WorldSpaceToBodySpace(const Vec3& p) const
	{
		Vec3 temp = p - GetCenterOfMassWorldSpace();
		Quat inverseOrient = orientation.Inverse();
		Vec3 bodySpace = inverseOrient.RotatePoint(temp);
		return bodySpace;
	}
	Vec3 Body::BodySpaceToWorldSpace(const Vec3& p) const
	{
		Vec3 worldSpace = GetCenterOfMassWorldSpace() + orientation.RotatePoint(p);
		return worldSpace;
	}
}