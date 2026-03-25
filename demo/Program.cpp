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
