#include "Contact.h"

namespace Cacti
{
	void ResolveContact(Contact& contact)
	{
		Body* a = contact.bodyA;
		Body* b = contact.bodyB;
		
		const float invMassA = a->invMass;
		const float invMassB = b->invMass;

		const Vec3 vab = a->linearVelocity - b->linearVelocity;
		const float impulseJ = -2.0f * vab.Dot(contact.normal) / (invMassA + invMassB);
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
