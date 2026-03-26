#pragma once
#include "World.h"
#include <TransformBuffer.h>
namespace Cacti
{
	class Engine
	{
	public:
		Engine() = default;
		~Engine() = default;

		World world;
		TransformBuffer transformBuffer;

	private:
	};

}
