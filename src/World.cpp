#include "World.h"
#include "Contact.h"
#include "./Physics/BroadPhase.h"
#include <iostream>
#include <cstdlib>
namespace Cacti
{
	World::World()
	{
		bodies.reserve(128);
	}

	void World::Reset()
	{
	}

	int CompareContacts(const void* p1, const void* p2) {
		Contact a = *(Contact*)p1;
		Contact b = *(Contact*)p2;

		if (a.timeOfImpact < b.timeOfImpact) {
			return -1;
		}

		if (a.timeOfImpact == b.timeOfImpact) {
			return 0;
		}

		return 1;
	}

	void World::Initialize()
	{
		Body body;
		for(int x = 0; x < 6; x++) {
			for(int z = 0; z < 6; z++) {
				float radius  = 0.5 ;
				float xx = float(x - 1) * radius * 1.5f;
				float zz = float(z - 1) * radius * 1.5f;
				body.position = Vec3(xx, 10, zz);
				body.orientation = Quat(0, 0, 0, 1);
				body.linearVelocity.Zero();
				body.invMass = 1.0f;
				body.elasticity = 0.5f;
				body.friction= 0.5f;
				body.shape = new Sphere(radius);
				bodies.push_back(body);
			}
		}

		for (int x = 0; x < 3; x++) {
			for (int z = 0; z < 3; z++) {
				float radius = 80;
				float xx = float(x - 1) * radius * 0.25f;
				float zz = float(z - 1) * radius * 0.25f;
				body.position = Vec3(xx, -radius, zz);
				body.orientation = Quat(0, 0, 0, 1);
				body.linearVelocity.Zero();
				body.invMass = 0.0f;
				body.elasticity = 0.99f;
				body.friction = 0.5f;
				body.shape = new Sphere(radius);
				bodies.push_back(body);
			}
		}
	}

	void World::Update(const float dt)
	{
		// Reset collided flags for this frame
		for (int i = 0; i < (int)bodies.size(); ++i) {
			bodies[i].shape->bounds.collided = false;
		}

		for (int i = 0; i < bodies.size(); i++)
		{
			float mass = 1 / bodies[i].invMass;
			Vec3 impulseGravity = Vec3(0.0f, -10, 0.0f) * mass * dt;
			bodies[i].ApplyImpulse(bodies[i].position, impulseGravity);
		}

		// Broadphase

		std::vector< CollisionPair > collisionPairs;
		BroadPhase(bodies.data(), (int)bodies.size(), collisionPairs, dt);


		int numContacts = 0;
		const int maxContacts = bodies.size() * bodies.size();
		std::vector<Contact> contacts(maxContacts);

		for (int i = 0; i < bodies.size(); i++)
		{
			for (int j = i + 1; j < bodies.size(); j++)
			{
				Body* a = &bodies[i];
				Body* b = &bodies[j];
				
				if (a->invMass == 0.0f && b->invMass == 0.0f)
				{
					continue;
				}
				Contact contact;
				if (Intersect(a, b, dt, contact))
				{
					contacts[numContacts] = contact;
					numContacts++;
				}
			}
		}

        if (numContacts > 1)
		{
			qsort(contacts.data(), numContacts, sizeof(Contact), CompareContacts);
		}

		float accumulatedTime = 0.0f;
		for (int i = 0; i < numContacts; i++) 
		{
			Contact& contact = contacts[i];
			const float dt = contact.timeOfImpact - accumulatedTime;

			// Position update
			for (int j = 0; j < bodies.size(); j++) {
				bodies[j].Update(dt);
			}

			ResolveContact(contact);
			accumulatedTime += dt;
		}

		const float timeRemaining = dt - accumulatedTime;
		if (timeRemaining > 0.0f) 
		{
			for (int i = 0; i < bodies.size(); i++) 
			{
				bodies[i].Update(timeRemaining);
			}
		}

	}

}
