#pragma once
#include <vector>
#include "Math/Vector.h"
#include "Math/Quat.h"
#include "Math/Bounds.h"
namespace Cacti
{
	class TransformBuffer
	{
	public:
		TransformBuffer() = default;

		~TransformBuffer() = default;

		std::vector<Vec3> positions;
		std::vector<Quat> orientations;
		std::vector<Bounds> boundingBoxes;
		void Resize(int count)
		{
			positions.resize(count);
			orientations.resize(count);
			boundingBoxes.resize(count);
		}

	private:
	};

}
