#pragma once
#include "raylib.h"

class Cam
{
public:
	Cam();
	~Cam() = default;

	void Update();
	Camera3D ReturnCam();
private:
	Camera3D cam;
};
