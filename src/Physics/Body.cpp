#include "Body.h"

namespace Cacti
{
	Body::Body()
		:position(0,0,0), linearVelocity(0,0,0)
	{
	}
	void Body::Update(const float dt)
	{
		position += linearVelocity * dt;

		Vec3 positionCM = GetCenterOfMassWorldSpace();
		Vec3 cmToPos = position - positionCM;

		Mat3 orientation = this->orientation.ToMat3();
		Mat3 inertiaTensor = orientation * shape->InertiaTensor() * orientation.Transpose();

		Vec3 alpha = inertiaTensor.Inverse() * (angularVelocity.Cross(inertiaTensor * angularVelocity));
		angularVelocity += alpha * dt;

		Vec3 dAngle = angularVelocity * dt;
		Quat dq = Quat(dAngle, dAngle.GetMagnitude());
		this->orientation = dq * this->orientation;
		this->orientation.Normalize();

		position = positionCM + dq.RotatePoint(cmToPos);
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
	Mat3 Body::GetInverseInertiaTensorBodySpace() const
	{
		Mat3 inertiaTensor = shape->InertiaTensor();
		Mat3 inverseInertiaTensor = inertiaTensor.Inverse() * invMass;
		return inverseInertiaTensor;
	}
	Mat3 Body::GetInverseInertiaTensorWorldSpace() const
	{
		Mat3 inertiaTensor = shape->InertiaTensor();
		Mat3 inverseInertiaTensor = inertiaTensor.Inverse() * invMass;
		Mat3 orient = orientation.ToMat3();
		inverseInertiaTensor = orient * inverseInertiaTensor * orient.Transpose();
		return inverseInertiaTensor;
	}
	void Body::ApplyImpulseAngular(Vec3 impulse)
	{
		if (invMass == 0.0f)
		{
			return;
		}

		// L = I w = r x p
		// dL = I dw = r x J
		// = > dw = I ^ −1 * (r x J)
		
		angularVelocity += GetInverseInertiaTensorWorldSpace() * impulse;
		const float maxAngularSpeed = 30.0f;
		if (angularVelocity.GetLengthSqr() > maxAngularSpeed * maxAngularSpeed)
		{
			angularVelocity.Normalize();
			angularVelocity *= maxAngularSpeed;
		}

	}
	void Body::ApplyImpulse(Vec3 impulsePoint, Vec3 impulse)
	{
		if (invMass == 0.0f)
		{
			return;
		}

		// impulse Point is the world space location of the application the impulse
		// impulse is the world space direction and magnitude of the impulse
		ApplyImpulseLinear(impulse);
		Vec3 position = GetCenterOfMassWorldSpace(); // applying impulses must produce to rques the center of mass
		Vec3 r = impulsePoint - position;
		Vec3 dL = r.Cross(impulse); // this is in world space.
		ApplyImpulseAngular(dL);
	}
	void Body::ApplyImpulseLinear(Vec3 impulse)
	{
		if (invMass == 0.0f)
		{
			return;
		}

		// p = mv
		// dp = m dv = J
		// = > dv = J / m
		linearVelocity += impulse * invMass;
	}
}