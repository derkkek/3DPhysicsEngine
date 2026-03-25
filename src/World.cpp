#include "World.h"
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
		body.position = Vec3(0, 0, 0);
		body.orientation = Quat(0, 0, 0, 1);
		body.shape = new Sphere(1.0f);
		bodies.push_back(body);

		body.position = Vec3(0, -101, 0);
		body.orientation = Quat(0, 0, 0, 1);
		body.shape = new Sphere(100.0f);
		bodies.push_back(body);
	}

	void World::Update(const float dt_sec)
	{
	}

}
