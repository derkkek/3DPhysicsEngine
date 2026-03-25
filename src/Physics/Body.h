#pragma once
#include "./Math/Vector.h"
#include "./Math/Quat.h"
#include "./Physics/ShapeBase.h"
namespace Cacti
{
	class Body
	{
	public:
		Body() = default;
		~Body() = default;

		Vec3 position;
		Quat orientation;
		Shape* shape;
	private:

	};

}
