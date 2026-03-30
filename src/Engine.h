#pragma once
#include "World.h"
#include <TransformBuffer.h>
namespace Cacti
{
	class Engine
	{
	public:
		Engine();
		~Engine() = default;
		void Update(float dt);
		void UpdateTransformBuffer(float dt);
		void Init();
		World world;
		TransformBuffer transformBuffer;

	private:
	};

}
