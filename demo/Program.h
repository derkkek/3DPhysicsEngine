#pragma once
#include "raylib.h"
#include <vector>
#include "Cam.h"
#include <string>
#include <unordered_map>
#include "Renderer.h"
#include <Engine.h>


class Program
{
public:
	Program();
	~Program() = default;

	void Update();
	void Destroy();
	void InitScene();
private:
	void Init();
	static RenderModel BuildRenderModelFromPhysicsGeometry(Cacti::Body body, Cacti::Shape* shape);

	Renderer renderer;
	Cacti::Engine engine;
	bool running;

	ConvertedSceneData convertedSceneData;
};
