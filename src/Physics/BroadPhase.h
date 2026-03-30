#pragma once
#include <vector>
#include "Body.h"


namespace Cacti
{
	struct PsuedoBody 
	{
		int id;
		float value;
		bool ismin;
	};

	struct CollisionPair
	{
		int a;
		int b;

		bool operator == (const CollisionPair& rhs) const {
			return (((a == rhs.a) && (b == rhs.b)) || ((a == rhs.b) && (b == rhs.a)));
		}
		bool operator != (const CollisionPair& rhs) const {
			return !(*this == rhs);
		}
	};

	void BroadPhase(const Body* bodies, const int num, std::vector< CollisionPair >& finalPairs, const float dt_sec);

}