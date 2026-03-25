#include "Program.h"


#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            100
#endif

#include "raymath.h"

Program::Program()
	:running(true)
{
	Init();
	renderer.Init();
	InitScene();
}

void Program::Init()
{
	const int screenWidth = 1800;
	const int screenHeight = 800;

	SetConfigFlags(FLAG_MSAA_4X_HINT);

	InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

	SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

	DisableCursor();

}

void Program::InitScene()
{
	Model cubeModel = LoadModelFromMesh(GenMeshCube(2.0f, 2.0f, 2.0f));
	Model planeModel = LoadModelFromMesh(GenMeshPlane(100.0f, 100.0f, 1, 1));

	RenderModel cube{ cubeModel,  BLUE,  Vector3{ 0.0f, 1.0f, 0.0f } };
	RenderModel plane{ planeModel, WHITE, Vector3Zero() };

	renderer.AddSceneObject(cube);
	renderer.AddSceneObject(plane);
}

void Program::Update()
{
	while (!WindowShouldClose())
	{
		float dt = GetFrameTime();

		renderer.Update();
	}
}


void Program::Destroy()
{

	CloseWindow();
}
