#include "Contact.h"

namespace Cacti
{
	void ResolveContact(Contact& contact)
	{
		Body* a = contact.bodyA;
		Body* b = contact.bodyB;
		
		const float invMassA = a->invMass;
		const float invMassB = b->invMass;

		const float elasticityA = a->elasticity;
		const float elasticityB = b->elasticity;

		const float elasticity = elasticityA * elasticityB;

		const Vec3 vab = a->linearVelocity - b->linearVelocity;
		const float impulseJ = -(1 + elasticity) * vab.Dot(contact.normal) / (invMassA + invMassB); //we solve collision only in the collision normal direction so we project relative velocity to collision normal. We don't need other directions.
		const Vec3 vectorImpulseJ = contact.normal * impulseJ;

		a->ApplyImpulse(vectorImpulseJ * 1.0f);
		b->ApplyImpulse(vectorImpulseJ * -1.0f);

		// also move colliding objects to just outside of each other.

		const float tA = a->invMass / (a->invMass + b->invMass);
		const float tB = b->invMass / (a->invMass + b->invMass);

		const Vec3 ds = contact.ptOnB_WorldSpace - contact.ptOnA_WorldSpace;

		a->position += ds * tA;
		b->position -= ds * tB;
	}

}
