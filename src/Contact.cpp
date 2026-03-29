#include "Contact.h"

namespace Cacti
{
	void ResolveContact(Contact& contact)
	{
		Body* a = contact.bodyA;
		Body* b = contact.bodyB;
		
		const Vec3 ptOnA = contact.ptOnA_WorldSpace;
		const Vec3 ptOnB = contact.ptOnB_WorldSpace;

		const float invMassA = a->invMass;
		const float invMassB = b->invMass;

		const float elasticityA = a->elasticity;
		const float elasticityB = b->elasticity;

		const float elasticity = elasticityA * elasticityB;

		const Mat3 invWorldInertiaA = a->GetInverseInertiaTensorBodySpace();
		const Mat3 invWorldInertiaB = b->GetInverseInertiaTensorBodySpace();

		const Vec3 n = contact.normal;

		const Vec3 ra = ptOnA - a->GetCenterOfMassWorldSpace();
		const Vec3 rb = ptOnB - b->GetCenterOfMassWorldSpace();

		const Vec3 angularJA = (invWorldInertiaA * ra.Cross(n)).Cross(ra);
		const Vec3 angularJB = (invWorldInertiaB * rb.Cross(n)).Cross(rb);

		const float angularFactor = (angularJA + angularJB).Dot(n);

		const Vec3 velA = a->linearVelocity + a->angularVelocity.Cross(ra);
		const Vec3 velB = b->linearVelocity + b->angularVelocity.Cross(rb);

		const Vec3 vab = velA - velB;
		const float impulseJ = (1 + elasticity) * vab.Dot(n) / (invMassA + invMassB); //we solve collision only in the collision normal direction so we project relative velocity to collision normal. We don't need other directions.
		const Vec3 vectorImpulseJ = n * impulseJ;

		a->ApplyImpulse(ptOnA, vectorImpulseJ * -1.0f);
		b->ApplyImpulse(ptOnB, vectorImpulseJ * 1.0f);

		const float frictionA = a->friction;
		const float frictionB = b->friction;
		const float friction = frictionA * frictionB;

		const Vec3 velNormal = n * n.Dot(vab);
		const Vec3 velTang = vab - velNormal;
		Vec3 relativeVelTang = velTang;
		relativeVelTang.Normalize();

		const Vec3 inertiaA = (invWorldInertiaA * ra.Cross(relativeVelTang)).Cross(ra);
		const Vec3 inertiaB = (invWorldInertiaB * rb.Cross(relativeVelTang)).Cross(rb);
		const float invInertia = (inertiaA + inertiaB).Dot(relativeVelTang);

		const float reducedMass = 1.0f / (a->invMass + b->invMass + invInertia);
		const Vec3 impulseFriction = velTang * reducedMass * friction;

		a->ApplyImpulse(ptOnA, impulseFriction * -1.0f);
		b->ApplyImpulse(ptOnB, impulseFriction * 1.0f);

		// also move colliding objects to just outside of each other.

		const Vec3 ds = ptOnB - ptOnA;

		const float tA = invMassA / (invMassA + invMassB);
		const float tB = invMassB / (invMassA + invMassB);

		a->position += ds * tA;
		b->position -= ds * tB;
	}

}
