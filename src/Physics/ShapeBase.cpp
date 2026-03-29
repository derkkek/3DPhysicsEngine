#include "ShapeBase.h"

namespace Cacti
{
	Mat3 Sphere::InertiaTensor() const
	{
		Mat3 tensor;
		tensor.Zero();
		tensor.rows[0][0] = 2.0f * radius * radius / 5.0f;
		tensor.rows[1][1] = 2.0f * radius * radius / 5.0f;
		tensor.rows[2][2] = 2.0f * radius * radius / 5.0f;
		return tensor;
	}
	Bounds Sphere::GetBounds(const Vec3& pos, const Quat& orient) const
	{
		Bounds temp;
		temp.mins = Vec3(-radius) + pos;
		temp.maxs = Vec3(radius) + pos;
		return temp;
	}
	Bounds Sphere::GetBounds() const
	{
		Bounds temp;
		temp.mins = Vec3(-radius);
		temp.maxs = Vec3(radius);
		return temp;
	}
}