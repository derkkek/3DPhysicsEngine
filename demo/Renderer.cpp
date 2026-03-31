#include "Renderer.h"
#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"
#include "rlgl.h"
#include "raymath.h"
#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            100
#endif
#include <random> 
#include <iostream>
Renderer::Renderer()
	:cam(),shader(),shadowShader()
{
}

TextureCubemap Renderer::GenTextureCubemap(Shader shader, Texture2D panorama, int size, int format)
{
	TextureCubemap cubemap = { 0 };

	rlDisableBackfaceCulling();     // Disable backface culling to render inside the cube

	// STEP 1: Setup framebuffer
	//------------------------------------------------------------------------------------------
	unsigned int rbo = rlLoadTextureDepth(size, size, true);
	cubemap.id = rlLoadTextureCubemap(0, size, format,1);

	unsigned int fbo = rlLoadFramebuffer();
	rlFramebufferAttach(fbo, rbo, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);
	rlFramebufferAttach(fbo, cubemap.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_CUBEMAP_POSITIVE_X, 0);

	// Check if framebuffer is complete with attachments (valid)
	if (rlFramebufferComplete(fbo)) TraceLog(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", fbo);
	//------------------------------------------------------------------------------------------

	// STEP 2: Draw to framebuffer
	//------------------------------------------------------------------------------------------
	// NOTE: Shader is used to convert HDR equirectangular environment map to cubemap equivalent (6 faces)
	rlEnableShader(shader.id);

	// Define projection matrix and send it to shader
	Matrix matFboProjection = MatrixPerspective(90.0 * DEG2RAD, 1.0, RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);

	rlSetUniformMatrix(shader.locs[SHADER_LOC_MATRIX_PROJECTION], matFboProjection);

	// Define view matrix for every side of the cubemap
	Matrix fboViews[6] = {
		MatrixLookAt(Vector3 { 0.0f, 0.0f, 0.0f }, Vector3 { 1.0f,  0.0f,  0.0f }, Vector3 { 0.0f, -1.0f,  0.0f }),
		MatrixLookAt(Vector3 { 0.0f, 0.0f, 0.0f }, Vector3 { -1.0f,  0.0f,  0.0f }, Vector3 { 0.0f, -1.0f,  0.0f }),
		MatrixLookAt(Vector3 { 0.0f, 0.0f, 0.0f }, Vector3 { 0.0f,  1.0f,  0.0f }, Vector3 { 0.0f,  0.0f,  1.0f }),
		MatrixLookAt(Vector3 { 0.0f, 0.0f, 0.0f }, Vector3 { 0.0f, -1.0f,  0.0f }, Vector3 { 0.0f,  0.0f, -1.0f }),
		MatrixLookAt(Vector3 { 0.0f, 0.0f, 0.0f }, Vector3 { 0.0f,  0.0f,  1.0f }, Vector3 { 0.0f, -1.0f,  0.0f }),
		MatrixLookAt(Vector3 { 0.0f, 0.0f, 0.0f }, Vector3 { 0.0f,  0.0f, -1.0f }, Vector3 { 0.0f, -1.0f,  0.0f })
	};

	rlViewport(0, 0, size, size);   // Set viewport to current fbo dimensions

	// Activate and enable texture for drawing to cubemap faces
	rlActiveTextureSlot(0);
	rlEnableTexture(panorama.id);

	for (int i = 0; i < 6; i++)
	{
		// Set the view matrix for the current cube face
		rlSetUniformMatrix(shader.locs[SHADER_LOC_MATRIX_VIEW], fboViews[i]);

		// Select the current cubemap face attachment for the fbo
		// WARNING: This function by default enables->attach->disables fbo!!!
		rlFramebufferAttach(fbo, cubemap.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_CUBEMAP_POSITIVE_X + i, 0);
		rlEnableFramebuffer(fbo);

		// Load and draw a cube, it uses the current enabled texture
		rlClearScreenBuffers();
		rlLoadDrawCube();

		// ALTERNATIVE: Try to use internal batch system to draw the cube instead of rlLoadDrawCube
		// for some reason this method does not work, maybe due to cube triangles definition? normals pointing out?
		// TODO: Investigate this issue...
		//rlSetTexture(panorama.id); // WARNING: It must be called after enabling current framebuffer if using internal batch system!
		//rlClearScreenBuffers();
		//DrawCubeV(Vector3Zero(), Vector3One(), WHITE);
		//rlDrawRenderBatchActive();
	}
	//------------------------------------------------------------------------------------------

	// STEP 3: Unload framebuffer and reset state
	//------------------------------------------------------------------------------------------
	rlDisableShader();          // Unbind shader
	rlDisableTexture();         // Unbind texture
	rlDisableFramebuffer();     // Unbind framebuffer
	rlUnloadFramebuffer(fbo);   // Unload framebuffer (and automatically attached depth texture/renderbuffer)

	// Reset viewport dimensions to default
	rlViewport(0, 0, rlGetFramebufferWidth(), rlGetFramebufferHeight());
	rlEnableBackfaceCulling();
	//------------------------------------------------------------------------------------------

	cubemap.width = size;
	cubemap.height = size;
	cubemap.mipmaps = 1;
	cubemap.format = format;

	return cubemap;
}

RenderTexture2D Renderer::LoadShadowmapRenderTexture(int width, int height)
{
	RenderTexture2D target = { 0 };
	target.id = rlLoadFramebuffer();
	target.texture.width = width;
	target.texture.height = height;

	if (target.id > 0)
	{
		rlEnableFramebuffer(target.id);
		target.depth.id = rlLoadTextureDepth(width, height, false);
		target.depth.width = width;
		target.depth.height = height;
		target.depth.format = 19;
		target.depth.mipmaps = 1;
		rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);
		if (rlFramebufferComplete(target.id))
			TraceLog(LOG_INFO, "FBO: [ID %i] Shadowmap framebuffer created successfully", target.id);
		rlDisableFramebuffer();
	}
	return target;
}

void Renderer::UnloadShadowmapRenderTexture(RenderTexture2D target)
{
	if (target.id > 0)
		rlUnloadFramebuffer(target.id);
}

void Renderer::Init()
{
	// --- Shadow shader ---
	shadowShader = LoadShader(
		TextFormat("resources/shaders/glsl%i/shadowmap.vs", GLSL_VERSION),
		TextFormat("resources/shaders/glsl%i/shadowmap.fs", GLSL_VERSION));

	shadowShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shadowShader, "viewPos");

	lightDir = Vector3Normalize({ 0.45f, -1.0f, -0.45f });
	Color lightColor = WHITE;
	Vector4 lightColorNorm = ColorNormalize(lightColor);

	lightDirLoc = GetShaderLocation(shadowShader, "lightDir");
	lightVPLoc = GetShaderLocation(shadowShader, "lightVP");
	shadowMapLoc = GetShaderLocation(shadowShader, "shadowMap");

	SetShaderValue(shadowShader, lightDirLoc, &lightDir, SHADER_UNIFORM_VEC3);
	SetShaderValue(shadowShader, GetShaderLocation(shadowShader, "lightColor"),
		&lightColorNorm, SHADER_UNIFORM_VEC4);

	float ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	SetShaderValue(shadowShader, GetShaderLocation(shadowShader, "ambient"),
		ambient, SHADER_UNIFORM_VEC4);

	int shadowMapRes = SHADOWMAP_RESOLUTION;
	SetShaderValue(shadowShader, GetShaderLocation(shadowShader, "shadowMapResolution"),
		&shadowMapRes, SHADER_UNIFORM_INT);

	// --- Shadow map FBO ---
	shadowMap = LoadShadowmapRenderTexture(SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION);

	// --- Skybox ---
	skyboxModel = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
	skyboxModel.materials[0].shader = LoadShader(
		TextFormat("resources/shaders/glsl%i/skybox.vs", GLSL_VERSION),
		TextFormat("resources/shaders/glsl%i/skybox.fs", GLSL_VERSION));

	int envMap = MATERIAL_MAP_CUBEMAP;
	SetShaderValue(skyboxModel.materials[0].shader,
		GetShaderLocation(skyboxModel.materials[0].shader, "environmentMap"),
		&envMap, SHADER_UNIFORM_INT);
	int doGamma = 0, vflipped = 0;
	SetShaderValue(skyboxModel.materials[0].shader,
		GetShaderLocation(skyboxModel.materials[0].shader, "doGamma"),
		&doGamma, SHADER_UNIFORM_INT);
	SetShaderValue(skyboxModel.materials[0].shader,
		GetShaderLocation(skyboxModel.materials[0].shader, "vflipped"),
		&vflipped, SHADER_UNIFORM_INT);

	Shader shdrCubemap = LoadShader(
		TextFormat("resources/shaders/glsl%i/cubemap.vs", GLSL_VERSION),
		TextFormat("resources/shaders/glsl%i/cubemap.fs", GLSL_VERSION));
	int equirect = 0;
	SetShaderValue(shdrCubemap,
		GetShaderLocation(shdrCubemap, "equirectangularMap"),
		&equirect, SHADER_UNIFORM_INT);

	Texture2D panorama = LoadTexture("resources/skybox.png");
	skyboxModel.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture =
		GenTextureCubemap(shdrCubemap, panorama, 1024, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
	UnloadTexture(panorama);
	UnloadShader(shdrCubemap);
}


void Renderer::Update()
{
	cam.Update();
	Camera3D camera = cam.ReturnCam();

	// Update view position uniform
	SetShaderValue(shadowShader, shadowShader.locs[SHADER_LOC_VECTOR_VIEW],
		&camera.position, SHADER_UNIFORM_VEC3);

	// -------------------------------------------------------
	// PASS 1: Render depth from light's point of view
	// -------------------------------------------------------
	Camera3D lightCamera = { 0 };
	lightCamera.position = Vector3Scale(lightDir, -15.0f);
	lightCamera.target = Vector3Zero();
	lightCamera.projection = CAMERA_ORTHOGRAPHIC;
	lightCamera.up = { 0.0f, 1.0f, 0.0f };
	lightCamera.fovy = 180.0f;

	Matrix lightView, lightProj;

	BeginTextureMode(shadowMap);
	ClearBackground(WHITE);
	BeginMode3D(lightCamera);
	lightView = rlGetMatrixModelview();
	lightProj = rlGetMatrixProjection();
	
	rlSetCullFace(RL_CULL_FACE_FRONT);
	for (int i = 0; i < sceneObjects.size(); i++)
	{
		const Vec3& p = transformBuffer->positions[i];
		const Cacti::Quat& q = transformBuffer->orientations[i];
		sceneObjects[i].position = { p.x, p.y, p.z };//Update positions from transform buffer.
		sceneObjects[i].orientation = {q.x, q.y, q.z, q.w};

		sceneObjects[i].Draw();
	}
	rlSetCullFace(RL_CULL_FACE_BACK);   // restore


	EndMode3D();
	EndTextureMode();

	Matrix lightViewProj = MatrixMultiply(lightView, lightProj);

	// -------------------------------------------------------
	// PASS 2: Render scene with shadow lookup
	// -------------------------------------------------------
	BeginDrawing();
	ClearBackground(BLACK);

	SetShaderValueMatrix(shadowShader, lightVPLoc, lightViewProj);

	rlEnableShader(shadowShader.id);
	rlActiveTextureSlot(textureActiveSlot);
	rlEnableTexture(shadowMap.depth.id);
	rlSetUniform(shadowMapLoc, &textureActiveSlot, SHADER_UNIFORM_INT, 1);

	BeginMode3D(camera);

	// Skybox
	rlDisableBackfaceCulling();
	rlDisableDepthMask();
	DrawModel(skyboxModel, camera.position, 1.0f, WHITE);
	rlEnableBackfaceCulling();
	rlEnableDepthMask();

	for (int i = 0; i < sceneObjects.size(); i++)
	{
		sceneObjects[i].Draw();

		BoundingBox bb{};
		bb.max.x = transformBuffer->boundingBoxes[i].maxs.x;
		bb.max.y = transformBuffer->boundingBoxes[i].maxs.y;
		bb.max.z = transformBuffer->boundingBoxes[i].maxs.z;

		bb.min.x = transformBuffer->boundingBoxes[i].mins.x;
		bb.min.y = transformBuffer->boundingBoxes[i].mins.y;
		bb.min.z = transformBuffer->boundingBoxes[i].mins.z;

		if (transformBuffer->boundingBoxes[i].collided)
		{
			DrawBoundingBox(bb, RED);
		}
		else
		{
			DrawBoundingBox(bb, GREEN);
		}
	}



	EndMode3D();
	DrawFPS(10, 10);
	EndDrawing();
}

void Renderer::Destroy()
{
	for (RenderModel& obj : sceneObjects)
		UnloadModel(obj.model);

	UnloadShader(shadowShader);
	UnloadShadowmapRenderTexture(shadowMap);
	UnloadShader(skyboxModel.materials[0].shader);
	UnloadTexture(skyboxModel.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture);
	UnloadModel(skyboxModel);
}

void Renderer::AddSceneObject(RenderModel& obj)
{
	obj.model.materials[0].shader = shadowShader;
	sceneObjects.emplace_back(obj);
}



Mesh RenderModel::CreatePolygonMesh(Vec3* corners, int count, Color color)
{
	Mesh mesh = {};
	if (count < 8) return mesh;

	mesh.vertexCount = 8;
	mesh.triangleCount = 12;

	mesh.vertices = (float*)RL_MALLOC(8 * 3 * sizeof(float));
	mesh.normals = (float*)RL_MALLOC(8 * 3 * sizeof(float));
	mesh.texcoords = (float*)RL_MALLOC(8 * 2 * sizeof(float));
	mesh.indices = (unsigned short*)RL_MALLOC(36 * sizeof(unsigned short));

	for (int i = 0; i < 8; i++)
	{
		mesh.vertices[3 * i] = corners[i].x;
		mesh.vertices[3 * i + 1] = corners[i].y;
		mesh.vertices[3 * i + 2] = corners[i].z;
		mesh.normals[3 * i] = 0.0f;
		mesh.normals[3 * i + 1] = 1.0f;
		mesh.normals[3 * i + 2] = 0.0f;
		mesh.texcoords[2 * i] = 0.0f;
		mesh.texcoords[2 * i + 1] = 0.0f;
	}

	unsigned short idx[] = {
		0,2,1,  1,2,3,   // bottom
		4,5,6,  5,7,6,   // top
		0,1,4,  1,5,4,   // front
		2,6,3,  3,6,7,   // back
		0,4,2,  2,4,6,   // left
		1,3,5,  3,7,5,   // right
	};
	memcpy(mesh.indices, idx, sizeof(idx));

	UploadMesh(&mesh, false);
	return mesh;
}


RenderModel RenderModel::BuildFromShape(Cacti::Body body, Cacti::Shape* shape)
{
	if (shape->GetType() == Cacti::Shape::ShapeType::SPHERE)
	{
		Cacti::Sphere* sphereShape = (Cacti::Sphere*)shape;
		Model sphere = LoadModelFromMesh(GenMeshSphere(sphereShape->radius, 50, 50));
		Vector3 raylibPos = { body.position.x, body.position.y, body.position.z };

		static const Color colorList[] = {
	   LIGHTGRAY, GRAY, DARKGRAY, YELLOW, GOLD, ORANGE, PINK, RED, MAROON,
	   GREEN, LIME, DARKGREEN, SKYBLUE, BLUE, DARKBLUE, PURPLE, VIOLET,
	   DARKPURPLE, BEIGE, BROWN, DARKBROWN, WHITE, BLACK, MAGENTA, RAYWHITE
		};
		static const size_t colorCount = sizeof(colorList) / sizeof(colorList[0]);

		static std::random_device rd;
		static std::mt19937 gen(rd());
		static std::uniform_int_distribution<size_t> dist(0, colorCount - 1);

		Color randomColor = colorList[dist(gen)];

		RenderModel sphereObj{ sphere, randomColor, raylibPos };


		return sphereObj;
	}

	else if (shape->GetType() == Cacti::Shape::ShapeType::CONVEX)
	{
		Cacti::Convex* convexShape = (Cacti::Convex*)shape;
		const Cacti::Bounds& b = convexShape->bounds;
		Vec3 corners[8] = {
			Vec3(b.mins.x, b.mins.y, b.mins.z),  // 0: -x -y -z
			Vec3(b.maxs.x, b.mins.y, b.mins.z),  // 1: +x -y -z
			Vec3(b.mins.x, b.mins.y, b.maxs.z),  // 2: -x -y +z
			Vec3(b.maxs.x, b.mins.y, b.maxs.z),  // 3: +x -y +z
			Vec3(b.mins.x, b.maxs.y, b.mins.z),  // 4: -x +y -z
			Vec3(b.maxs.x, b.maxs.y, b.mins.z),  // 5: +x +y -z
			Vec3(b.mins.x, b.maxs.y, b.maxs.z),  // 6: -x +y +z
			Vec3(b.maxs.x, b.maxs.y, b.maxs.z),  // 7: +x +y +z
		};
		static const Color colorList[] = {
LIGHTGRAY, GRAY, DARKGRAY, YELLOW, GOLD, ORANGE, PINK, RED, MAROON,
GREEN, LIME, DARKGREEN, SKYBLUE, BLUE, DARKBLUE, PURPLE, VIOLET,
DARKPURPLE, BEIGE, BROWN, DARKBROWN, WHITE, BLACK, MAGENTA, RAYWHITE
		};
		static const size_t colorCount = sizeof(colorList) / sizeof(colorList[0]);

		static std::random_device rd;
		static std::mt19937 gen(rd());
		static std::uniform_int_distribution<size_t> dist(0, colorCount - 1);

		Color randomColor = colorList[dist(gen)];


		Model polygon = LoadModelFromMesh(RenderModel::CreatePolygonMesh(corners, 8, randomColor));
		Vector3 raylibPos = { body.position.x, body.position.y, body.position.z };
		RenderModel polyObj{ polygon, randomColor, raylibPos };

		return polyObj;
	}

	return RenderModel();
}


RenderModel::RenderModel(Model& model, Color color, Vector3 pos)
	:model(model), color(color), position(pos)
{

}

void RenderModel::Draw()
{
	Vector3 axis;
	float angle;
 	//orientation.ToAxisAngle(axis, angle);
	QuaternionToAxisAngle(orientation, &axis, &angle);
	float angleDeg = angle * RAD2DEG;
	Vector3 raylibAxis = { axis.x, axis.y, axis.z };

	DrawModelEx(this->model, this->position, raylibAxis, angleDeg, Vector3One(), this->color);
}

