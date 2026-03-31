#include "ShapesDataBase.h"


namespace Cacti
{
	static const float w = 50;
	static const float h = 25;

	Vec3 BoxGround[] = {
		Vec3(-w,0, -h),
		Vec3(w,0, -h),
		Vec3(-w, 0, h),
		Vec3(w, 0, h),

		Vec3(-w,-1, -h),
		Vec3(w,-1, -h),
		Vec3(-w, -1, h),
		Vec3(w, -1, h),
	};

	Vec3 BoxWall0[] = {
		Vec3(-1,0, -h),
		Vec3(1,0, -h),
		Vec3(-1, 0, h),
		Vec3(1, 0, h),

		Vec3(-1,5, -h),
		Vec3(1,5, -h),
		Vec3(-1, 5, h),
		Vec3(1, 5, h),
	};

	Vec3 BoxWall1[] = {
		Vec3(-w,0, -1),
		Vec3(w,0, -1),
		Vec3(-w, 0, 1),
		Vec3(w, 0, 1),

		Vec3(-w,5, -1),
		Vec3(w,5, -1),
		Vec3(-w, 5, 1),
		Vec3(w, 5, 1),
	};

	Vec3 BoxUnit[] = {
		Vec3(-1,-1,-1),
		Vec3(1,-1,-1),
		Vec3(-1, -1,1),
		Vec3(1, -1,1),

		Vec3(-1,1, -1),
		Vec3(1,1, -1),
		Vec3(-1, 1, 1),
		Vec3(1, 1, 1),
	};

	static const float t = 0.25f;
	Vec3 BoxSmall[] = {
		Vec3(-t,-t,-t),
		Vec3(t,-t,-t),
		Vec3(-t, -t,t),
		Vec3(t, -t,t),

		Vec3(-t,t, -t),
		Vec3(t,t, -t),
		Vec3(-t, t, t),
		Vec3(t, t, t),
	};

	static const float l = 3.0f;
	Vec3 BoxBeam[] = {
		Vec3(-l,-t,-t),
		Vec3(l,-t,-t),
		Vec3(-l, -t,t),
		Vec3(l, -t,t),

		Vec3(-l,t, -t),
		Vec3(l,t, -t),
		Vec3(-l, t, t),
		Vec3(l, t, t),
	};

	Vec3 BoxPlatform[] = {
		Vec3(-l,-t,-l),
		Vec3(l,-t,-l),
		Vec3(-l, -t,l),
		Vec3(l, -t,l),

		Vec3(-l,t, -l),
		Vec3(l,t, -l),
		Vec3(-l, t, l),
		Vec3(l, t, l),
	};

	static const float t2 = 0.25f;
	static const float w2 = t2 * 2.0f;
	static const float h3 = t2 * 4.0f;
	Vec3 BoxBody[] = {
		Vec3(-t2,-h3,-w2),
		Vec3(t2,-h3,-w2),
		Vec3(-t2, -h3,w2),
		Vec3(t2, -h3,w2),

		Vec3(-t2,h3, -w2),
		Vec3(t2,h3, -w2),
		Vec3(-t2, h3, w2),
		Vec3(t2, h3, w2),
	};

	static const float h2 = 0.25f;
	Vec3 BoxLimb[] = {
		Vec3(-h3,-h2,-h2),
		Vec3(h3,-h2,-h2),
		Vec3(-h3, -h2,h2),
		Vec3(h3, -h2,h2),

		Vec3(-h3,h2, -h2),
		Vec3(h3,h2, -h2),
		Vec3(-h3, h2, h2),
		Vec3(h3, h2, h2),
	};

	Vec3 BoxHead[] = {
		Vec3(-h2,-h2,-h2),
		Vec3(h2,-h2,-h2),
		Vec3(-h2, -h2,h2),
		Vec3(h2, -h2,h2),

		Vec3(-h2,h2, -h2),
		Vec3(h2,h2, -h2),
		Vec3(-h2, h2, h2),
		Vec3(h2, h2, h2),
	};

	Vec3 Diamond[7 * 8];
	void FillDiamond() {
		Vec3 pts[4 + 4];
		pts[0] = Vec3(0.1f, -1, 0);
		pts[1] = Vec3(1, 0, 0);
		pts[2] = Vec3(1, 0.1f, 0);
		pts[3] = Vec3(0.4f, 0.4f, 0);

		const float pi = acosf(-1.0f);
		const Quat quatHalf(Vec3(0, 1, 0), 2.0f * pi * 0.125f * 0.5f);
		pts[4] = Vec3(0.8f, 0.3, 0);
		pts[4] = quatHalf.RotatePoint(pts[4]);
		pts[5] = quatHalf.RotatePoint(pts[1]);
		pts[6] = quatHalf.RotatePoint(pts[2]);

		const Quat quat(Vec3(0, 1, 0), 2.0f * pi * 0.125f);
		int idx = 0;
		for (int i = 0; i < 7; i++) {
			Diamond[idx] = pts[i];
			idx++;
		}

		Quat quatAccumulator;
		for (int i = 1; i < 8; i++) {
			quatAccumulator = quatAccumulator * quat;
			for (int pt = 0; pt < 7; pt++) {
				Diamond[idx] = quatAccumulator.RotatePoint(pts[pt]);
				idx++;
			}
		}
	}
}
