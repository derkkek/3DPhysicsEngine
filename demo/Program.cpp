#include "Program.h"


#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            100
#endif

#include "raymath.h"

Program::Program()
	:running(true), engine()
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

	DisableCursor();

}

void Program::InitScene()
{
	engine.world.Initialize();

	renderer.sceneObjects.reserve(engine.world.bodies.size());

	for (int i = 0; i < engine.world.bodies.size(); i++) 
	{
		RenderModel* sceneObject = RenderModel::BuildFromShape(engine.world.bodies[i], engine.world.bodies[i].shape);
		renderer.AddSceneObject(sceneObject);
	}



}

void Program::Update()
{
	while (!WindowShouldClose())
	{
		float dt = GetFrameTime();
		engine.world.Update(dt);
		//for (int i = 0; i < renderer.sceneObjects.size(); i++)
		//{
		//	renderer.UpdateRenderModelData(engine.world.bodies[i], i);
		//}
		renderer.Update();
	}
}


void Program::Destroy()
{
	for (RenderModel* obj : renderer.sceneObjects)
	{
		delete obj;
	}
	CloseWindow();
}
