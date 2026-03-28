#include "Engine.h"
namespace Cacti
{
	Engine::Engine()
	{
		transformBuffer.positions.reserve(world.MAX_BODIES);
		transformBuffer.orientations.reserve(world.MAX_BODIES);
	}
	void Engine::Init()
	{
		transformBuffer.Resize(world.MAX_BODIES);
	}
	void Engine::Update(float dt)
	{
		world.Update(dt);
		UpdateTransformBuffer();
	}
	void Engine::UpdateTransformBuffer()
	{
		for (int i = 0; i < world.bodies.size(); i++)
		{
			transformBuffer.positions[i] = world.bodies[i].position;
			transformBuffer.orientations[i] = world.bodies[i].orientation;
		}
	}
}
