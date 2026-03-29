#pragma once
#include <vector>
#include "Body.h"


namespace Cacti
{
	struct psuedoBody 
	{
		int id;
		float value;
		bool ismin;
	};

	struct collisionPair
	{
		int a;
		int b;

		bool operator == (const collisionPair& rhs) const {
			return (((a == rhs.a) && (b == rhs.b)) || ((a == rhs.b) && (b == rhs.a)));
		}
		bool operator != (const collisionPair& rhs) const {
			return !(*this == rhs);
		}
	};

	void BroadPhase(const Body* bodies, const int num, std::vector< collisionPair >& finalPairs, const float dt_sec);

}