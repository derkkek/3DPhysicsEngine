#include "ShapeBase.h"
namespace Cacti
{

	Vec3 Sphere::Support(const Vec3& dir, const Vec3& pos, const Quat& orient, const float bias) const
	{
		return (pos + dir * (radius + bias));
	}
	Mat3 Sphere::InertiaTensor() const
	{
		Mat3 tensor;
		tensor.Zero();
		tensor.rows[0][0] = 2.0f * radius * radius / 5.0f;
		tensor.rows[1][1] = 2.0f * radius * radius / 5.0f;
		tensor.rows[2][2] = 2.0f * radius * radius / 5.0f;
		return tensor;
	}
	Bounds Sphere::GetBounds(const Vec3& pos, const Quat& orient) const
	{
		Bounds temp;
		temp.mins = Vec3(-radius) + pos;
		temp.maxs = Vec3(radius) + pos;
		return temp;
	}
	Bounds Sphere::GetBounds() const
	{
		Bounds temp;
		temp.mins = Vec3(-radius);
		temp.maxs = Vec3(radius);
		return temp;
	}

	void Box::Build(const Vec3* pts, const int num)
	{
		for (int i = 0; i < num; i++)
		{
			bounds.Expand(pts[i]);
		}

		points.clear();
		points.push_back(Vec3(bounds.mins.x, bounds.mins.y, bounds.mins.z));
		points.push_back(Vec3(bounds.maxs.x, bounds.mins.y, bounds.mins.z));
		points.push_back(Vec3(bounds.mins.x, bounds.maxs.y, bounds.mins.z));
		points.push_back(Vec3(bounds.mins.x, bounds.mins.y, bounds.maxs.z));
		points.push_back(Vec3(bounds.maxs.x, bounds.maxs.y, bounds.maxs.z));
		points.push_back(Vec3(bounds.mins.x, bounds.maxs.y, bounds.maxs.z));
		points.push_back(Vec3(bounds.maxs.x, bounds.mins.y, bounds.maxs.z));
		points.push_back(Vec3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z));
		centerOfMass = (bounds.maxs + bounds.mins) * 0.5f;
	}
	Vec3 Box::Support(const Vec3& dir, const Vec3& pos, const Quat& orient, const float bias) const
	{
		// Find the point in furthest in direction
		Vec3 maxPt = orient.RotatePoint(points[0]) + pos;
		float maxDist = dir.Dot(maxPt);
		for (int i = 1; i < points.size(); i++) {
			const Vec3 pt = orient.RotatePoint(points[i]) + pos;
			const float dist = dir.Dot(pt);

			if (dist > maxDist) {
				maxDist = dist;
				maxPt = pt;
			}
		}

		Vec3 norm = dir;
		norm.Normalize();
		norm *= bias;

		return maxPt + norm;
	}
	Mat3 Box::InertiaTensor() const
	{
		// Inertia tensor for box centered around zero
		const float dx = bounds.maxs.x - bounds.mins.x;
		const float dy = bounds.maxs.y - bounds.mins.y;
		const float dz = bounds.maxs.z - bounds.mins.z;

		Mat3 tensor;
		tensor.Zero();
		tensor.rows[0][0] = (dy * dy + dz * dz) / 12.0f;
		tensor.rows[1][1] = (dx * dx + dz * dz) / 12.0f;
		tensor.rows[2][2] = (dx * dx + dy * dy) / 12.0f;

		// Now we need to use the parallel axis theorem to get the inertia tensor for a box
		// that is not centered around the origin

		Vec3 cm;
		cm.x = (bounds.maxs.x + bounds.mins.x) * 0.5f;
		cm.y = (bounds.maxs.y + bounds.mins.y) * 0.5f;
		cm.z = (bounds.maxs.z + bounds.mins.z) * 0.5f;

		const Vec3 R = Vec3(0, 0, 0) - cm;	// the displacement from center of mass to the origin
		const float R2 = R.GetLengthSqr();
		Mat3 patTensor;
		patTensor.rows[0] = Vec3(R2 - R.x * R.x, R.x * R.y, R.x * R.z);
		patTensor.rows[1] = Vec3(R.y * R.x, R2 - R.y * R.y, R.y * R.z);
		patTensor.rows[2] = Vec3(R.z * R.x, R.z * R.y, R2 - R.z * R.z);

		// Now we need to add the center of mass tensor and the parallel axis theorem tensor together;
		tensor += patTensor;
		return tensor;
	}
	Bounds Box::GetBounds(const Vec3& pos, const Quat& orient) const
	{
		Vec3 corners[8];
		corners[0] = Vec3(bounds.mins.x, bounds.mins.y, bounds.mins.z);
		corners[1] = Vec3(bounds.mins.x, bounds.mins.y, bounds.maxs.z);
		corners[2] = Vec3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);
		corners[3] = Vec3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);

		corners[4] = Vec3(bounds.maxs.x, bounds.maxs.y, bounds.maxs.z);
		corners[5] = Vec3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);
		corners[6] = Vec3(bounds.maxs.x, bounds.mins.y, bounds.maxs.z);
		corners[7] = Vec3(bounds.mins.x, bounds.maxs.y, bounds.maxs.z);

		Bounds bounds;
		for (int i = 0; i < 8; i++) {
			corners[i] = orient.RotatePoint(corners[i]) + pos;
			bounds.Expand(corners[i]);
		}

		return bounds;
	}
	float Box::FastestLinearSpeed(const Vec3& angularVelocity, const Vec3& dir) const 
	{
		float maxSpeed = 0.0f;
		for (int i = 0; i < points.size(); i++) {
			Vec3 r = points[i] - centerOfMass;
			Vec3 linearVelocity = angularVelocity.Cross(r);
			float speed = dir.Dot(linearVelocity);
			if (speed > maxSpeed) {
				maxSpeed = speed;
			}
		}
		return maxSpeed;
	}
	Bounds Convex::GetBounds(const Vec3& pos, const Quat& orient) const
	{
		Vec3 corners[8];
		corners[0] = Vec3(bounds.mins.x, bounds.mins.y, bounds.mins.z);
		corners[1] = Vec3(bounds.mins.x, bounds.mins.y, bounds.maxs.z);
		corners[2] = Vec3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);
		corners[3] = Vec3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);

		corners[4] = Vec3(bounds.maxs.x, bounds.maxs.y, bounds.maxs.z);
		corners[5] = Vec3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);
		corners[6] = Vec3(bounds.maxs.x, bounds.mins.y, bounds.maxs.z);
		corners[7] = Vec3(bounds.mins.x, bounds.maxs.y, bounds.maxs.z);

		Bounds bounds;
		for (int i = 0; i < 8; i++) {
			corners[i] = orient.RotatePoint(corners[i]) + pos;
			bounds.Expand(corners[i]);
		}

		return bounds;
	}
	float Convex::FastestLinearSpeed(const Vec3& angularVelocity, const Vec3& dir) const
	{
		float maxSpeed = 0.0f;
		for (int i = 0; i < points.size(); i++) {
			Vec3 r = points[i] - centerOfMass;
			Vec3 linearVelocity = angularVelocity.Cross(r);
			float speed = dir.Dot(linearVelocity);
			if (speed > maxSpeed) {
				maxSpeed = speed;
			}
		}
		return maxSpeed;
	}





	//Following functions are Convex shape functions.


	int FindPointFurthestInDir(const Vec3* pts, const int num, const Vec3& dir) {
		int maxIdx = 0;
		float maxDist = dir.Dot(pts[0]);
		for (int i = 1; i < num; i++) {
			float dist = dir.Dot(pts[i]);
			if (dist > maxDist) {
				maxDist = dist;
				maxIdx = i;
			}
		}
		return maxIdx;
	}


	float DistanceFromLine(const Vec3& a, const Vec3& b, const Vec3& pt) {
		Vec3 ab = b - a;
		ab.Normalize();

		Vec3 ray = pt - a;
		Vec3 projection = ab * ray.Dot(ab);	// project the ray onto ab
		Vec3 perpindicular = ray - projection;
		return perpindicular.GetMagnitude();
	}

	Vec3 FindPointFurthestFromLine(const Vec3* pts, const int num, const Vec3& ptA, const Vec3& ptB) {
		int maxIdx = 0;
		float maxDist = DistanceFromLine(ptA, ptB, pts[0]);
		for (int i = 1; i < num; i++) {
			float dist = DistanceFromLine(ptA, ptB, pts[i]);
			if (dist > maxDist) {
				maxDist = dist;
				maxIdx = i;
			}
		}
		return pts[maxIdx];
	}

	float DistanceFromTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& pt) {
		Vec3 ab = b - a;
		Vec3 ac = c - a;
		Vec3 normal = ab.Cross(ac);
		normal.Normalize();

		Vec3 ray = pt - a;
		float dist = ray.Dot(normal);
		return dist;
	}


	Vec3 FindPointFurthestFromTriangle(const Vec3* pts, const int num, const Vec3& ptA, const Vec3& ptB, const Vec3& ptC) {
		int maxIdx = 0;
		float maxDist = DistanceFromTriangle(ptA, ptB, ptC, pts[0]);
		for (int i = 1; i < num; i++) {
			float dist = DistanceFromTriangle(ptA, ptB, ptC, pts[i]);
			if (dist * dist > maxDist * maxDist) {
				maxDist = dist;
				maxIdx = i;
			}
		}
		return pts[maxIdx];
	}


	void BuildTetrahedron(const Vec3* verts, const int num, std::vector< Vec3 >& hullPts, std::vector< Cacti::tri >& hullTris) {
		hullPts.clear();
		hullTris.clear();

		Vec3 points[4];

		int idx = FindPointFurthestInDir(verts, num, Vec3(1, 0, 0));
		points[0] = verts[idx];
		idx = FindPointFurthestInDir(verts, num, points[0] * -1.0f);
		points[1] = verts[idx];
		points[2] = FindPointFurthestFromLine(verts, num, points[0], points[1]);
		points[3] = FindPointFurthestFromTriangle(verts, num, points[0], points[1], points[2]);

		// This is important for making sure the ordering is CCW for all faces.
		float dist = DistanceFromTriangle(points[0], points[1], points[2], points[3]);
		if (dist > 0.0f) {
			std::swap(points[0], points[1]);
		}

		// Build the tetrahedron
		hullPts.push_back(points[0]);
		hullPts.push_back(points[1]);
		hullPts.push_back(points[2]);
		hullPts.push_back(points[3]);

		tri tri;
		tri.a = 0;
		tri.b = 1;
		tri.c = 2;
		hullTris.push_back(tri);

		tri.a = 0;
		tri.b = 2;
		tri.c = 3;
		hullTris.push_back(tri);

		tri.a = 2;
		tri.b = 1;
		tri.c = 3;
		hullTris.push_back(tri);

		tri.a = 1;
		tri.b = 0;
		tri.c = 3;
		hullTris.push_back(tri);
	}

	void RemoveInternalPoints(const std::vector< Vec3 >& hullPoints, const std::vector< tri >& hullTris, std::vector< Vec3 >& checkPts) {
		for (int i = 0; i < checkPts.size(); i++) {
			const Vec3& pt = checkPts[i];

			bool isExternal = false;
			for (int t = 0; t < hullTris.size(); t++) {
				const tri& tri = hullTris[t];
				const Vec3& a = hullPoints[tri.a];
				const Vec3& b = hullPoints[tri.b];
				const Vec3& c = hullPoints[tri.c];

				// If the point is in front of any triangle then it's external
				float dist = DistanceFromTriangle(a, b, c, pt);
				if (dist > 0.0f) {
					isExternal = true;
					break;
				}
			}

			// if it's not external, then it's inside the polyhedron and should be removed
			if (!isExternal) {
				checkPts.erase(checkPts.begin() + i);
				i--;
			}
		}

		// Also remove any points that are just a little too close to the hull points
		for (int i = 0; i < checkPts.size(); i++) {
			const Vec3& pt = checkPts[i];

			bool isTooClose = false;
			for (int j = 0; j < hullPoints.size(); j++) {
				Vec3 hullPt = hullPoints[j];
				Vec3 ray = hullPt - pt;
				if (ray.GetLengthSqr() < 0.01f * 0.01f) {	// 1cm is too close
					isTooClose = true;
					break;
				}
			}

			if (isTooClose) {
				checkPts.erase(checkPts.begin() + i);
				i--;
			}
		}
	}

	bool IsEdgeUnique(const std::vector< tri >& tris, const std::vector< int >& facingTris, const int ignoreTri, const Edge& edge) {
		for (int i = 0; i < facingTris.size(); i++) {
			const int triIdx = facingTris[i];
			if (ignoreTri == triIdx) {
				continue;
			}

			const tri& tri = tris[triIdx];

			Edge edges[3];
			edges[0].a = tri.a;
			edges[0].b = tri.b;

			edges[1].a = tri.b;
			edges[1].b = tri.c;

			edges[2].a = tri.c;
			edges[2].b = tri.a;

			for (int e = 0; e < 3; e++) {
				if (edge == edges[e]) {
					return false;
				}
			}
		}
		return true;
	}

	void AddPoint(std::vector< Vec3 >& hullPoints, std::vector< tri >& hullTris, const Vec3& pt) {
		// This point is outside
		// Now we need to remove old triangles and build new ones

		// Find all the triangles that face this point
		std::vector< int > facingTris;
		for (int i = (int)hullTris.size() - 1; i >= 0; i--) {
			const tri& tri = hullTris[i];

			const Vec3& a = hullPoints[tri.a];
			const Vec3& b = hullPoints[tri.b];
			const Vec3& c = hullPoints[tri.c];

			const float dist = DistanceFromTriangle(a, b, c, pt);
			if (dist > 0.0f) {
				facingTris.push_back(i);
			}
		}

		// Now find all edges that are unique to the tris, these will be the edges that form the new triangles
		std::vector< Edge > uniqueEdges;
		for (int i = 0; i < facingTris.size(); i++) {
			const int triIdx = facingTris[i];
			const tri& tri = hullTris[triIdx];

			Edge edges[3];
			edges[0].a = tri.a;
			edges[0].b = tri.b;

			edges[1].a = tri.b;
			edges[1].b = tri.c;

			edges[2].a = tri.c;
			edges[2].b = tri.a;

			for (int e = 0; e < 3; e++) {
				if (IsEdgeUnique(hullTris, facingTris, triIdx, edges[e])) {
					uniqueEdges.push_back(edges[e]);
				}
			}
		}

		// now remove the old facing tris
		for (int i = 0; i < facingTris.size(); i++) {
			hullTris.erase(hullTris.begin() + facingTris[i]);
		}

		// Now add the new point
		hullPoints.push_back(pt);
		const int newPtIdx = (int)hullPoints.size() - 1;

		// Now add triangles for each unique edge
		for (int i = 0; i < uniqueEdges.size(); i++) {
			const Edge& edge = uniqueEdges[i];

			tri tri;
			tri.a = edge.a;
			tri.b = edge.b;
			tri.c = newPtIdx;
			hullTris.push_back(tri);
		}
	}

	void RemoveUnreferencedVerts(std::vector< Vec3 >& hullPoints, std::vector< tri >& hullTris) {
		for (int i = 0; i < hullPoints.size(); i++) {

			bool isUsed = false;
			for (int j = 0; j < hullTris.size(); j++) {
				const tri& tri = hullTris[j];

				if (tri.a == i || tri.b == i || tri.c == i) {
					isUsed = true;
					break;
				}
			}

			if (isUsed) {
				continue;
			}

			for (int j = 0; j < hullTris.size(); j++) {
				tri& tri = hullTris[j];
				if (tri.a > i) {
					tri.a--;
				}
				if (tri.b > i) {
					tri.b--;
				}
				if (tri.c > i) {
					tri.c--;
				}
			}

			hullPoints.erase(hullPoints.begin() + i);
			i--;
		}
	}


	void ExpandConvexHull(std::vector< Vec3 >& hullPoints, std::vector< tri >& hullTris, const std::vector< Vec3 >& verts) {
		std::vector< Vec3 > externalVerts = verts;
		RemoveInternalPoints(hullPoints, hullTris, externalVerts);

		while (externalVerts.size() > 0) {
			int ptIdx = FindPointFurthestInDir(externalVerts.data(), (int)externalVerts.size(), externalVerts[0]);

			Vec3 pt = externalVerts[ptIdx];

			// remove this element
			externalVerts.erase(externalVerts.begin() + ptIdx);

			AddPoint(hullPoints, hullTris, pt);

			RemoveInternalPoints(hullPoints, hullTris, externalVerts);
		}

		RemoveUnreferencedVerts(hullPoints, hullTris);
	}

	bool IsExternal(const std::vector< Vec3 >& pts, const std::vector< tri >& tris, const Vec3& pt) {
		bool isExternal = false;
		for (int t = 0; t < tris.size(); t++) {
			const tri& tri = tris[t];
			const Vec3& a = pts[tri.a];
			const Vec3& b = pts[tri.b];
			const Vec3& c = pts[tri.c];

			// If the point is in front of any triangle then it's external
			float dist = DistanceFromTriangle(a, b, c, pt);
			if (dist > 0.0f) {
				isExternal = true;
				break;
			}
		}

		return isExternal;
	}

	Vec3 CalculateCenterOfMass(const std::vector< Vec3 >& pts, const std::vector< tri >& tris) {
		const int numSamples = 100;

		Bounds bounds;
		bounds.Expand(pts.data(), pts.size());

		Vec3 cm(0.0f);
		const float dx = bounds.WidthX() / (float)numSamples;
		const float dy = bounds.WidthY() / (float)numSamples;
		const float dz = bounds.WidthZ() / (float)numSamples;

		int sampleCount = 0;
		for (float x = bounds.mins.x; x < bounds.maxs.x; x += dx) {
			for (float y = bounds.mins.y; y < bounds.maxs.y; y += dy) {
				for (float z = bounds.mins.z; z < bounds.maxs.z; z += dz) {
					Vec3 pt(x, y, z);

					if (IsExternal(pts, tris, pt)) {
						continue;
					}

					cm += pt;
					sampleCount++;
				}
			}
		}

		cm /= (float)sampleCount;
		return cm;
	}

	Mat3 CalculateInertiaTensor(const std::vector< Vec3 >& pts, const std::vector< tri >& tris, const Vec3& cm) {
		const int numSamples = 100;

		Bounds bounds;
		bounds.Expand(pts.data(), (int)pts.size());

		Mat3 tensor;
		tensor.Zero();

		const float dx = bounds.WidthX() / (float)numSamples;
		const float dy = bounds.WidthY() / (float)numSamples;
		const float dz = bounds.WidthZ() / (float)numSamples;

		int sampleCount = 0;
		for (float x = bounds.mins.x; x < bounds.maxs.x; x += dx) {
			for (float y = bounds.mins.y; y < bounds.maxs.y; y += dy) {
				for (float z = bounds.mins.z; z < bounds.maxs.z; z += dz) {
					Vec3 pt(x, y, z);

					if (IsExternal(pts, tris, pt)) {
						continue;
					}

					// Get the point relative to the center of mass
					pt -= cm;

					tensor.rows[0][0] += pt.y * pt.y + pt.z * pt.z;
					tensor.rows[1][1] += pt.z * pt.z + pt.x * pt.x;
					tensor.rows[2][2] += pt.x * pt.x + pt.y * pt.y;

					tensor.rows[0][1] += -1.0f * pt.x * pt.y;
					tensor.rows[0][2] += -1.0f * pt.x * pt.z;
					tensor.rows[1][2] += -1.0f * pt.y * pt.z;

					tensor.rows[1][0] += -1.0f * pt.x * pt.y;
					tensor.rows[2][0] += -1.0f * pt.x * pt.z;
					tensor.rows[2][1] += -1.0f * pt.y * pt.z;

					sampleCount++;
				}
			}
		}

		tensor *= 1.0f / (float)sampleCount;
		return tensor;
	}

	void Cacti::BuildConvexHull(const std::vector< Vec3 >& verts, std::vector< Vec3 >& hullPts, std::vector< tri >& hullTris) {
		if (verts.size() < 4) {
			return;
		}

		// Build a tetrahedron
		BuildTetrahedron(verts.data(), (int)verts.size(), hullPts, hullTris);

		ExpandConvexHull(hullPts, hullTris, verts);
	}
	Vec3 Convex::Support(const Vec3& dir, const Vec3& pos, const Quat& orient, const float bias) const {
		// Find the point in furthest in direction
		Vec3 maxPt = orient.RotatePoint(points[0]) + pos;
		float maxDist = dir.Dot(maxPt);
		for (int i = 1; i < points.size(); i++) {
			const Vec3 pt = orient.RotatePoint(points[i]) + pos;
			const float dist = dir.Dot(pt);

			if (dist > maxDist) {
				maxDist = dist;
				maxPt = pt;
			}
		}

		Vec3 norm = dir;
		norm.Normalize();
		norm *= bias;

		return maxPt + norm;
	}

	void Convex::Build(const Vec3* pts, const int num) {
		points.clear();
		points.reserve(num);
		for (int i = 0; i < num; i++) {
			points.push_back(pts[i]);
		}

		// Expand into a convex hull
		std::vector< Vec3 > hullPoints;
		std::vector< tri > hullTriangles;
		BuildConvexHull(points, hullPoints, hullTriangles);
		points = hullPoints;

		// Expand the bounds
		bounds.Clear();
		bounds.Expand(points.data(), points.size());

		centerOfMass = CalculateCenterOfMass(hullPoints, hullTriangles);

		inertiaTensor = CalculateInertiaTensor(hullPoints, hullTriangles, centerOfMass);
	}

}