#pragma once
#include "Physics/ShapeBase.h"

namespace Cacti
{
	extern Vec3 BoxGround[8];
	extern Vec3 BoxWall0[8];
	extern Vec3 BoxWall1[8];
	extern Vec3 BoxUnit[8];
	extern Vec3 BoxSmall[8];
	extern Vec3 BoxBeam[8];
	extern Vec3 BoxPlatform[8];
	extern Vec3 BoxBody[8];
	extern Vec3 BoxLimb[8];
	extern Vec3 BoxHead[8];
	extern Vec3 Diamond[7 * 8];
	void FillDiamond();
}
