#pragma once
#include <vector>
#include "./Physics/Body.h"
#include "./Physics/ShapeBase.h"
#include "./Math/Vector.h"
#include "./Math/Quat.h"
namespace Cacti
{
	class World
	{
	public:
		World();
		~World() = default;

		void Reset();
		void Initialize();
		void Update(const float dt);

		std::vector<Body> bodies;

		const int MAX_BODIES = 256;
	private:


	};

}
