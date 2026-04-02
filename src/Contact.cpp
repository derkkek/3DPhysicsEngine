#include "Contact.h"

namespace Cacti
{
	void ResolveContact(Contact& contact) {
		Body* bodyA = contact.bodyA;
		Body* bodyB = contact.bodyB;

		const Vec3 ptOnA = bodyA->BodySpaceToWorldSpace(contact.ptOnA_LocalSpace);
		const Vec3 ptOnB = bodyB->BodySpaceToWorldSpace(contact.ptOnB_LocalSpace);

		const float elasticityA = bodyA->elasticity;
		const float elasticityB = bodyB->elasticity;
		const float elasticity = elasticityA * elasticityB;

		const float invMassA = bodyA->invMass;
		const float invMassB = bodyB->invMass;

		const Mat3 invWorldInertiaA = bodyA->GetInverseInertiaTensorWorldSpace();
		const Mat3 invWorldInertiaB = bodyB->GetInverseInertiaTensorWorldSpace();

		const Vec3 n = contact.normal;

		const Vec3 ra = ptOnA - bodyA->GetCenterOfMassWorldSpace();
		const Vec3 rb = ptOnB - bodyB->GetCenterOfMassWorldSpace();

		const Vec3 angularJA = (invWorldInertiaA * ra.Cross(n)).Cross(ra);
		const Vec3 angularJB = (invWorldInertiaB * rb.Cross(n)).Cross(rb);
		const float angularFactor = (angularJA + angularJB).Dot(n);

		// Get the world space velocity of the motion and rotation
		const Vec3 velA = bodyA->linearVelocity + bodyA->angularVelocity.Cross(ra);
		const Vec3 velB = bodyB->linearVelocity + bodyB->angularVelocity.Cross(rb);

		// Calculate the collision impulse
		const Vec3 vab = velA - velB;
		const float ImpulseJ = (1.0f + elasticity) * vab.Dot(n) / (invMassA + invMassB + angularFactor);
		const Vec3 vectorImpulseJ = n * ImpulseJ;

		bodyA->ApplyImpulse(ptOnA, vectorImpulseJ * -1.0f);
		bodyB->ApplyImpulse(ptOnB, vectorImpulseJ * 1.0f);

		//
		// Calculate the impulse caused by friction
		//

		const float frictionA = bodyA->friction;
		const float frictionB = bodyB->friction;
		const float friction = frictionA * frictionB;

		// Find the normal direction of the velocity with respect to the normal of the collision
		const Vec3 velNorm = n * n.Dot(vab);

		// Find the tangent direction of the velocity with respect to the normal of the collision
		const Vec3 velTang = vab - velNorm;

		// Get the tangential velocities relative to the other body
		Vec3 relativeVelTang = velTang;
		relativeVelTang.Normalize();

		const Vec3 inertiaA = (invWorldInertiaA * ra.Cross(relativeVelTang)).Cross(ra);
		const Vec3 inertiaB = (invWorldInertiaB * rb.Cross(relativeVelTang)).Cross(rb);
		const float invInertia = (inertiaA + inertiaB).Dot(relativeVelTang);

		// Calculate the tangential impulse for friction
		const float reducedMass = 1.0f / (bodyA->invMass + bodyB->invMass + invInertia);
		const Vec3 impulseFriction = velTang * reducedMass * friction;

		// Apply kinetic friction
		bodyA->ApplyImpulse(ptOnA, impulseFriction * -1.0f);
		bodyB->ApplyImpulse(ptOnB, impulseFriction * 1.0f);

		//
		// Let's also move our colliding objects to just outside of each other (projection method)
		//
		if (0.0f == contact.timeOfImpact) {
			const Vec3 ds = ptOnB - ptOnA;

			const float tA = invMassA / (invMassA + invMassB);
			const float tB = invMassB / (invMassA + invMassB);

			bodyA->position += ds * tA;
			bodyB->position -= ds * tB;
		}
	}

}
