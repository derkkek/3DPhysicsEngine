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

	void World::AddStandardSandBox(std::vector< Body >& bodies) {
		Body body;

		body.position = Vec3(0, 0, 0);
		body.orientation = Quat(0, 0, 0, 1);
		body.linearVelocity.Zero();
		body.angularVelocity.Zero();
		body.invMass = 0.0f;
		body.elasticity = 0.5f;
		body.friction = 0.5f;
		body.shape = new Box(BoxGround, sizeof(BoxGround) / sizeof(Vec3));
		bodies.push_back(body);

		body.position = Vec3(50, 0, 0);
		body.orientation = Quat(0, 0, 0, 1);
		body.linearVelocity.Zero();
		body.angularVelocity.Zero();
		body.invMass = 0.0f;
		body.elasticity = 0.5f;
		body.friction = 0.0f;
		body.shape = new Box(BoxWall0, sizeof(BoxWall0) / sizeof(Vec3));
		bodies.push_back(body);

		body.position = Vec3(-50, 0, 0);
		body.orientation = Quat(0, 0, 0, 1);
		body.linearVelocity.Zero();
		body.angularVelocity.Zero();
		body.invMass = 0.0f;
		body.elasticity = 0.5f;
		body.friction = 0.0f;
		body.shape = new Box(BoxWall0, sizeof(BoxWall0) / sizeof(Vec3));
		bodies.push_back(body);

		body.position = Vec3(0, 25, 0);
		body.orientation = Quat(0, 0, 0, 1);
		body.linearVelocity.Zero();
		body.angularVelocity.Zero();
		body.invMass = 0.0f;
		body.elasticity = 0.5f;
		body.friction = 0.0f;
		body.shape = new Box(BoxWall1, sizeof(BoxWall1) / sizeof(Vec3));
		bodies.push_back(body);

		body.position = Vec3(0, -25, 0);
		body.orientation = Quat(0, 0, 0, 1);
		body.linearVelocity.Zero();
		body.angularVelocity.Zero();
		body.invMass = 0.0f;
		body.elasticity = 0.5f;
		body.friction = 0.0f;
		body.shape = new Box(BoxWall1, sizeof(BoxWall1) / sizeof(Vec3));
		bodies.push_back(body);
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

		//body.position = Vec3(10, 3, 0);
		//body.orientation = Quat(0, 0, 0, 1);
		//body.linearVelocity = Vec3(0, 0, 0);
		//body.angularVelocity = Vec3(0.0f, 0.0f, 0.0f);
		//body.invMass = 1.0f;
		//body.elasticity = 0.5f;
		//body.friction = 0.5f;
		//body.shape = new Sphere(0.5f);
		//bodies.push_back(body);

		body.position = Vec3(0, 0, 0);
		body.orientation = Quat(0, 0, 0, 1);
		body.linearVelocity = Vec3(0, 0, 0);
		body.angularVelocity = Vec3(0, 0, 0);
		body.invMass = 1.0f;
		body.elasticity = 0.5f;
		body.friction = 0.5f;
		FillDiamond();
		body.shape = new Convex(Cacti::Diamond, sizeof(Cacti::Diamond) / sizeof(Vec3));
		bodies.push_back(body);

		//AddStandardSandBox(bodies);
	}

	void World::Update(const float dt)
	{

		for (int i = 0; i < bodies.size(); i++)
		{
			float mass = 1 / bodies[i].invMass;
			Vec3 impulseGravity = Vec3(0.0f, -10, 0.0f) * mass * dt;
			bodies[i].shape->bounds.collided = false;
			//bodies[i].ApplyImpulse(bodies[i].position, impulseGravity);
		}

		// Broadphase

		std::vector< CollisionPair > collisionPairs;
		BroadPhase(bodies.data(), (int)bodies.size(), collisionPairs, dt);


		int numContacts = 0;
		const int maxContacts = bodies.size() * bodies.size();
		std::vector<Contact> contacts(maxContacts);

		for (int i = 0; i < collisionPairs.size(); i++) 
		{
			const CollisionPair& pair = collisionPairs[i];
			Body* bodyA = &bodies[pair.a];
			Body* bodyB = &bodies[pair.b];

			// Skip body pairs with infinite mass
			if (0.0f == bodyA->invMass && 0.0f == bodyB->invMass) {
				continue;
			}

			Contact contact;
			if (Intersect(bodyA, bodyB, dt, contact)) {
				contacts[numContacts] = contact;
				numContacts++;
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
