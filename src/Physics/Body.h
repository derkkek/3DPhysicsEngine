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

		void Update(const float dt);

		Vec3 GetCenterOfMassWorldSpace() const;
		Vec3 GetCenterOfMassModelSpace() const;

		Vec3 WorldSpaceToBodySpace(const Vec3& p) const;
		Vec3 BodySpaceToWorldSpace(const Vec3& p) const;

		Mat3 GetInverseInertiaTensorBodySpace() const;
		Mat3 GetInverseInertiaTensorWorldSpace() const;

		void ApplyImpulse(Vec3 impulsePoint, Vec3 impulse);
		void ApplyImpulseLinear(Vec3 impulse);
		void ApplyImpulseAngular(Vec3 impulse);

		Vec3 linearVelocity;
		Vec3 angularVelocity;
		Vec3 position;
		Quat orientation;
		Shape* shape;

		float elasticity;
		float friction;
		float invMass;
	private:

	};

}
