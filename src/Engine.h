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
		void UpdateTransformBuffer();
		void Init();
		World world;
		TransformBuffer transformBuffer;

	private:
	};

}
