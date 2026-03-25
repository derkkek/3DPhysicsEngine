#pragma once

#include "raylib.h"

#include "rlights.h"

#include <vector>
#include <unordered_map>
#include "Cam.h"
#include <string>
#include "Physics/ShapeBase.h"
#include "Physics/Body.h"

#define SHADOWMAP_RESOLUTION 2048

class RenderModel
{
public:
	Model model;
	Color color;
	static RenderModel* BuildFromShape(Cacti::Body body,Cacti::Shape* shape);

	Vector3 position;

	RenderModel() = default;
	RenderModel(Model& model, Color color, Vector3 pos);
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

	void AddSceneObject(RenderModel* obj);

	std::vector<RenderModel* > sceneObjects;


private:


	TextureCubemap GenTextureCubemap(Shader shader, Texture2D panorama, int size, int format);

	RenderTexture2D LoadShadowmapRenderTexture(int width, int height);
	void UnloadShadowmapRenderTexture(RenderTexture2D target);

	Shader shadowShader;
	int lightVPLoc;
	int shadowMapLoc;

	Vector3 lightDir;
	int lightDirLoc;

	RenderTexture2D shadowMap;
	int textureActiveSlot = 10;

	Cam cam;
	Shader shader;

	Model skyboxModel;
	Shader skyboxShader;


};
