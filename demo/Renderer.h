#pragma once

#include "raylib.h"

#include "rlights.h"

#include <vector>
#include <unordered_map>
#include "Cam.h"
#include <string>

class RenderModel
{
public:
	Model model;
	Color color;
	// TODO: void BuildFromShape(chai3d::Shape* shape);

	RenderModel(Model& model, Color color);
	void Draw();
};

class Renderer
{
public:
	Renderer();
	~Renderer() = default;

	void Init();
	void Update();
	void Destroy();

private:
	TextureCubemap GenTextureCubemap(Shader shader, Texture2D panorama, int size, int format);

	std::vector<RenderModel> sceneObjects;
	Cam cam;
	Shader shader;
	std::vector<Light> lights;

	Model skyboxModel;
	Shader skyboxShader;

	//std::unordered_map<std::string, Texture2D> textures;

};
