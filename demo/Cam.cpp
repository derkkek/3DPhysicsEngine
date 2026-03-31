#include "Cam.h"

Cam::Cam()
{
    cam = Camera3D{
        .position = Vector3{ 5.0f, 5.0f, 5.0f },
        .target = Vector3{ 0.0f, 0.0f, 0.0f },
        .up = Vector3{ 0.0f, 1.0f, 0.0f },
        .fovy = 45.0f,
        .projection = CAMERA_PERSPECTIVE
    };
}

void Cam::Update()
{
    UpdateCamera(&cam, CAMERA_FREE);
}

Camera3D Cam::ReturnCam()
{
    return cam;
}
