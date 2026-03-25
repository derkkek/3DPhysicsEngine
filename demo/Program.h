#pragma once
#include "raylib.h"
#include <vector>
#include "Cam.h"
#include <string>
#include <unordered_map>
#include "Renderer.h"

class Program
{
public:
	Program();
	~Program() = default;

	void Update();
	void Destroy();

private:
	void Init();
	
	//chai2d::Engine engine;
	Renderer renderer;

	bool running;

};
