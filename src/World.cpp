#include "World.h"
#include <iostream>
namespace Cacti
{
	World::World()
	{
		bodies.reserve(128);
	}

	void World::Reset()
	{
	}

	void World::Initialize()
	{
		Body body;
		body.position = Vec3(0, 5, 0);
		body.linearVelocity = Vec3(0, 0, 0);
		body.orientation = Quat(0, 0, 0, 1);
		body.invMass = 1.0f;
		body.elasticity = 0.5f;
		body.shape = new Sphere(1.0f);
		bodies.push_back(body);

		body.position = Vec3(0, -101, 0);
		body.orientation = Quat(0, 0, 0, 1);
		body.linearVelocity = Vec3(0, 0, 0);
		body.elasticity = 1.0f;
		body.invMass = 0.0f;
		body.shape = new Sphere(100.0f);
		bodies.push_back(body);
	}

	void World::Update(const float dt)
	{
		for (int i = 0; i < bodies.size(); i++)
		{
			float mass = 1 / bodies[i].invMass;
			Vec3 impulseGravity = Vec3(0, -10, 0) * mass * dt;
			bodies[i].ApplyImpulse(impulseGravity);
		}
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
				if (Intersect(a, b, contact))
				{
					ResolveContact(contact);
				}
			}
		}

		for (int i = 0; i < bodies.size(); i++)
		{
			bodies[i].position += bodies[i].linearVelocity * dt;
		}

	}

}
