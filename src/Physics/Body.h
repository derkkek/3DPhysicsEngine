#pragma once
#include "./Math/Vector.h"
#include "./Math/Quat.h"
#include "./Physics/ShapeBase.h"
namespace Cacti
{
	class Body
	{
	public:
		Body();
		~Body() = default;

		Vec3 GetCenterOfMassWorldSpace() const;
		Vec3 GetCenterOfMassModelSpace() const;

		Vec3 WorldSpaceToBodySpace(const Vec3& p) const;
		Vec3 BodySpaceToWorldSpace(const Vec3& p) const;

		void ApplyImpulse(Vec3 impules);

		Vec3 linearVelocity;
		Vec3 position;
		Quat orientation;
		Shape* shape;

		float elasticity;
		float invMass;

	private:

	};

}
