#pragma once
#include <vector>
#include "Math/Vector.h"
#include "Math/Quat.h"
namespace Cacti
{
	class TransformBuffer
	{
	public:
		TransformBuffer()
		{
			positions.reserve(engine.world.bodies.size());
		}
		~TransformBuffer() = default;

		std::vector<Vec3> positions;
		std::vector<Quat> orientations;

		void Resize(int count)
		{
			positions.resize(count);
			positions.resize(count);
		}

	private:
		Engine engine;
	};

}
