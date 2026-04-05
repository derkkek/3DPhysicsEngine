#pragma once

#include "raylib.h"

#include "rlights.h"

#include <vector>
#include <unordered_map>
#include <string>

#define SHADOWMAP_RESOLUTION 2048

struct ConvertedSceneData
{
	std::vector<Vector3> positions;
	std::vector<Quaternion> orientations;
	std::vector<BoundingBox> bbs;
	std::vector<bool> bbIndexCollided;
};


class RenderModel
{
public:
	Model model;
	Color color;

	RenderModel() = default;
	RenderModel(Model& model, Color color, Vector3 pos);
	void Draw(const Vector3 pos, const Quaternion& orient);
};


class Renderer
{
public:
	Renderer();
	~Renderer() = default;

	void Init();
	void Update(ConvertedSceneData& convertedSceneData);
	void Destroy();

	void AddSceneObject(RenderModel& obj);

	std::vector<RenderModel > sceneObjects;

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

	Camera3D cam;
	Shader shader;

	Model skyboxModel;
	Shader skyboxShader;
};
