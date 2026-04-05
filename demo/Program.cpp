#include "Program.h"


#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            100
#endif

#include "raymath.h"
#include <random>


Program::Program()
	:running(true), engine(), convertedSceneData{}
{
	Init();
	engine.Init();
	renderer.Init();
	InitScene();
}

void Program::Init()
{
	const int screenWidth = 1800;
	const int screenHeight = 800;

	SetConfigFlags(FLAG_MSAA_4X_HINT);

	InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

	DisableCursor();

}


void Program::InitScene()
{
	engine.world.Initialize();

	renderer.sceneObjects.reserve(engine.world.bodies.size());

	convertedSceneData.positions.resize(engine.transformBuffer.positions.size());
	convertedSceneData.orientations.resize(engine.transformBuffer.orientations.size());

	for (int i = 0; i < engine.world.bodies.size(); i++) 
	{
		RenderModel sceneObject = BuildRenderModelFromPhysicsGeometry(engine.world.bodies[i], engine.world.bodies[i].shape);
		renderer.AddSceneObject(sceneObject);
	}

}

void Program::Update()
{
	float elapsed = 0.0f;
	const float startDelay = 2.0f;

	while (!WindowShouldClose())
	{
		float dt = GetFrameTime();
		elapsed += dt;

		if (elapsed > startDelay)
		{
			engine.Update(dt);
		}

		for (int i = 0; i < engine.transformBuffer.positions.size(); i++)
		{
			const Vec3 p = engine.transformBuffer.positions[i];
			const Cacti::Quat q = engine.transformBuffer.orientations[i];
			const Cacti::Bounds boundingBox = engine.transformBuffer.boundingBoxes[i];

			BoundingBox bb{};

			bb.max.x = engine.transformBuffer.boundingBoxes[i].maxs.x;
			bb.max.y = engine.transformBuffer.boundingBoxes[i].maxs.y;
			bb.max.z = engine.transformBuffer.boundingBoxes[i].maxs.z;

			bb.min.x = engine.transformBuffer.boundingBoxes[i].mins.x;
			bb.min.y = engine.transformBuffer.boundingBoxes[i].mins.y;
			bb.min.z = engine.transformBuffer.boundingBoxes[i].mins.z;

			convertedSceneData.positions[i] = { p.x, p.y, p.z };
			convertedSceneData.orientations[i] = { q.x, q.y , q.z, q.w };
			convertedSceneData.bb[i] = { bb };
			convertedSceneData.bbIndexCollided[i] = engine.transformBuffer.boundingBoxes[i].collided;
		}

		renderer.Update(convertedSceneData);
	}
}


void Program::Destroy()
{
	CloseWindow();
}


RenderModel Program::BuildRenderModelFromPhysicsGeometry(Cacti::Body body, Cacti::Shape* shape)
{
	static const Color colorList[] = {
	LIGHTGRAY, GRAY, DARKGRAY, YELLOW, GOLD, ORANGE, PINK, RED, MAROON,
	GREEN, LIME, DARKGREEN, SKYBLUE, BLUE, DARKBLUE, PURPLE, VIOLET,
	DARKPURPLE, BEIGE, BROWN, DARKBROWN, WHITE, BLACK, MAGENTA, RAYWHITE
	};
	static const size_t colorCount = sizeof(colorList) / sizeof(colorList[0]);

	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_int_distribution<size_t> dist(0, colorCount - 1);




	if (shape->GetType() == Cacti::Shape::ShapeType::SPHERE)
	{
		Cacti::Sphere* sphereShape = (Cacti::Sphere*)shape;
		Model sphere = LoadModelFromMesh(GenMeshSphere(sphereShape->radius, 50, 50));
		Vector3 raylibPos = { body.position.x, body.position.y, body.position.z };
		Color randomColor = colorList[dist(gen)];
		RenderModel sphereObj{ sphere, randomColor, raylibPos };


		return sphereObj;
	}

	else if (shape->GetType() == Cacti::Shape::CONVEX) {
		const Cacti::Convex* shapeConvex = (const Cacti::Convex*)shape;

		// Build the convex hull from the points
		std::vector<Vec3> hullPts;
		std::vector<Cacti::tri> hullTris;
		Cacti::BuildConvexHull(shapeConvex->points, hullPts, hullTris);

		// Calculate smoothed normals per vertex
		std::vector<Vec3> normals;
		normals.reserve(hullPts.size());
		for (int i = 0; i < (int)hullPts.size(); i++) {
			Vec3 norm(0.0f);
			for (int t = 0; t < (int)hullTris.size(); t++) {
				const Cacti::tri& tri = hullTris[t];
				if (i != tri.a && i != tri.b && i != tri.c)
					continue;
				const Vec3& a = hullPts[tri.a];
				const Vec3& b = hullPts[tri.b];
				const Vec3& c = hullPts[tri.c];
				Vec3 ab = b - a;
				Vec3 ac = c - a;
				norm += ab.Cross(ac);
			}
			norm.Normalize();
			normals.push_back(norm);
		}

		int vertCount = (int)hullPts.size();
		int indexCount = (int)hullTris.size() * 3;

		Mesh mesh = {};
		mesh.vertexCount = vertCount;
		mesh.triangleCount = (int)hullTris.size();

		mesh.vertices = (float*)RL_MALLOC(vertCount * 3 * sizeof(float));
		mesh.normals = (float*)RL_MALLOC(vertCount * 3 * sizeof(float));
		mesh.texcoords = (float*)RL_MALLOC(vertCount * 2 * sizeof(float));
		mesh.indices = (unsigned short*)RL_MALLOC(indexCount * sizeof(unsigned short));

		for (int i = 0; i < vertCount; i++) {
			mesh.vertices[i * 3 + 0] = hullPts[i].x;
			mesh.vertices[i * 3 + 1] = hullPts[i].y;
			mesh.vertices[i * 3 + 2] = hullPts[i].z;

			mesh.normals[i * 3 + 0] = normals[i].x;
			mesh.normals[i * 3 + 1] = normals[i].y;
			mesh.normals[i * 3 + 2] = normals[i].z;

			mesh.texcoords[i * 2 + 0] = 0.0f;
			mesh.texcoords[i * 2 + 1] = 0.0f;
		}

		for (int i = 0; i < (int)hullTris.size(); i++) {
			mesh.indices[i * 3 + 0] = (unsigned short)hullTris[i].a;
			mesh.indices[i * 3 + 1] = (unsigned short)hullTris[i].b;
			mesh.indices[i * 3 + 2] = (unsigned short)hullTris[i].c;
		}

		UploadMesh(&mesh, false);
		Model model = LoadModelFromMesh(mesh);
		Color randomColor = colorList[dist(gen)];
		Vector3 raylibPos = { body.position.x, body.position.y, body.position.z };
		return RenderModel(model, randomColor, raylibPos);
	}
	else if (shape->GetType() == Cacti::Shape::BOX)
	{
		const Cacti::Box* shapeBox = (const Cacti::Box*)shape;

		// use the physics box's actual 8 corner points directly
		// this guarantees the mesh matches the physics shape exactly
		Vec3 mn = shapeBox->bounds.mins;
		Vec3 mx = shapeBox->bounds.maxs;

		// 8 corners of the box in local space
		Vec3 corners[8] = {
			Vec3(mn.x, mn.y, mn.z),
			Vec3(mx.x, mn.y, mn.z),
			Vec3(mn.x, mx.y, mn.z),
			Vec3(mx.x, mx.y, mn.z),
			Vec3(mn.x, mn.y, mx.z),
			Vec3(mx.x, mn.y, mx.z),
			Vec3(mn.x, mx.y, mx.z),
			Vec3(mx.x, mx.y, mx.z),
		};

		// 6 faces, each as 2 triangles = 12 triangles = 36 indices
		// each face has 4 unique vertices for correct normals
		// 6 faces * 4 verts = 24 vertices total
		int vertCount = 24;
		int indexCount = 36;

		Mesh mesh = {};
		mesh.vertexCount = vertCount;
		mesh.triangleCount = 12;
		mesh.vertices = (float*)RL_MALLOC(vertCount * 3 * sizeof(float));
		mesh.normals = (float*)RL_MALLOC(vertCount * 3 * sizeof(float));
		mesh.texcoords = (float*)RL_MALLOC(vertCount * 2 * sizeof(float));
		mesh.indices = (unsigned short*)RL_MALLOC(indexCount * sizeof(unsigned short));

		// face definitions: each face = 4 corner indices + outward normal
		struct Face {
			int c[4];       // indices into corners[]
			Vec3 normal;
		};

		Face faces[6] = {
			{ {4,5,7,6}, Vec3(0, 0, 1) }, // front  +Z
			{ {0,2,3,1}, Vec3(0, 0,-1) }, // back   -Z
			{ {2,6,7,3}, Vec3(0, 1, 0) }, // top    +Y
			{ {0,1,5,4}, Vec3(0,-1, 0) }, // bottom -Y
			{ {1,3,7,5}, Vec3(1, 0, 0) }, // right  +
			{ {0,4,6,2}, Vec3(-1, 0, 0) }, // left   -X
		};



		for (int f = 0; f < 6; f++)
		{
			int baseVert = f * 4;

			for (int v = 0; v < 4; v++)
			{
				Vec3 p = corners[faces[f].c[v]];
				int idx = (baseVert + v) * 3;
				mesh.vertices[idx + 0] = p.x;
				mesh.vertices[idx + 1] = p.y;
				mesh.vertices[idx + 2] = p.z;

				mesh.normals[idx + 0] = faces[f].normal.x;
				mesh.normals[idx + 1] = faces[f].normal.y;
				mesh.normals[idx + 2] = faces[f].normal.z;

				int uvIdx = (baseVert + v) * 2;
				mesh.texcoords[uvIdx + 0] = (v == 1 || v == 3) ? 1.0f : 0.0f;
				mesh.texcoords[uvIdx + 1] = (v == 2 || v == 3) ? 1.0f : 0.0f;
			}

			// two triangles per face
			int baseIdx = f * 6;
			mesh.indices[baseIdx + 0] = baseVert + 0;
			mesh.indices[baseIdx + 1] = baseVert + 1;
			mesh.indices[baseIdx + 2] = baseVert + 2;
			mesh.indices[baseIdx + 3] = baseVert + 0;
			mesh.indices[baseIdx + 4] = baseVert + 2;
			mesh.indices[baseIdx + 5] = baseVert + 3;
		}

		UploadMesh(&mesh, false);
		Model model = LoadModelFromMesh(mesh);

		Color randomColor = colorList[dist(gen)];

		Vector3 raylibPos = { body.position.x, body.position.y, body.position.z };
		return RenderModel(model, randomColor, raylibPos);
	}

	return RenderModel();
}