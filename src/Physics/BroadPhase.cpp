#include "BroadPhase.h"
#include "Body.h"
#include "Math/Vector.h"

namespace Cacti
{
	int CompareSAP(const void* a, const void* b) {
		const psuedoBody* ea = (const psuedoBody*)a;
		const psuedoBody* eb = (const psuedoBody*)b;

		if (ea->value < eb->value) {
			return -1;
		}
		return 1;
	}

	/*
	====================================================
	SortBodiesBounds
	====================================================
	*/
	void SortBodiesBounds(const Body* bodies, const int num, psuedoBody* sortedArray, const float dt_sec) {
		Vec3 axis = Vec3(1, 1, 1);
		axis.Normalize();

		for (int i = 0; i < num; i++) {
			const Body& body = bodies[i];
			Bounds bounds = body.shape->GetBounds(body.position, body.orientation);

			// Expand the bounds by the linear velocity
			bounds.Expand(bounds.mins + body.linearVelocity * dt_sec);
			bounds.Expand(bounds.maxs + body.linearVelocity * dt_sec);

			const float epsilon = 0.01f;
			bounds.Expand(bounds.mins + Vec3(-1, -1, -1) * epsilon);
			bounds.Expand(bounds.maxs + Vec3(1, 1, 1) * epsilon);

			sortedArray[i * 2 + 0].id = i;
			sortedArray[i * 2 + 0].value = axis.Dot(bounds.mins);
			sortedArray[i * 2 + 0].ismin = true;

			sortedArray[i * 2 + 1].id = i;
			sortedArray[i * 2 + 1].value = axis.Dot(bounds.maxs);
			sortedArray[i * 2 + 1].ismin = false;
		}

		qsort(sortedArray, num * 2, sizeof(psuedoBody), CompareSAP);
	}

	void BuildPairs(std::vector< collisionPair >& collisionPairs, const psuedoBody* sortedBodies, const int num) {
		collisionPairs.clear();

		// Now that the bodies are sorted, build the collision pairs
		for (int i = 0; i < num * 2; i++) {
			const psuedoBody& a = sortedBodies[i];
			if (!a.ismin) {
				continue;
			}

			collisionPair pair;
			pair.a = a.id;

			for (int j = i + 1; j < num * 2; j++) {
				const psuedoBody& b = sortedBodies[j];
				// if we've hit the end of the a element, then we're done creating pairs with a
				if (b.id == a.id) {
					break;
				}

				if (!b.ismin) {
					continue;
				}

				pair.b = b.id;
				collisionPairs.push_back(pair);
				
			}
		}
	}

	void SweepAndPrune1D(const Body* bodies, const int num, std::vector< collisionPair >& finalPairs, const float dt_sec) {
		psuedoBody* sortedBodies = (psuedoBody*)alloca(sizeof(psuedoBody) * num * 2);

		SortBodiesBounds(bodies, num, sortedBodies, dt_sec);
		BuildPairs(finalPairs, sortedBodies, num);
	}

	void BroadPhase(const Body* bodies, const int num, std::vector< collisionPair >& finalPairs, const float dt_sec) {
		finalPairs.clear();

		SweepAndPrune1D(bodies, num, finalPairs, dt_sec);

		// Flagging collision of bounding boxes to debug rendering
		for (const collisionPair& cp : finalPairs) {
			const int a = cp.a;
			const int b = cp.b;
			//if (a < 0 || a >= (int)bodies.size() || b < 0 || b >= (int)bodies.size()) continue;

			// get world-space bounds for each body
			Bounds ba = bodies[a].shape->GetBounds(bodies[a].position, bodies[a].orientation);
			Bounds bb = bodies[b].shape->GetBounds(bodies[b].position, bodies[b].orientation);

			if (ba.DoesIntersect(bb)) {
				bodies[a].shape->bounds.collided = true;
				bodies[b].shape->bounds.collided = true;
			}
		}
	}


}