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
	//DrawRay(Ray(Vector3{ 0,0,0 }, Vector3{ 1, 0, 0 }), RED);
	//DrawRay(Ray(Vector3{ 0,0,0 }, Vector3{ 0, 1, 0 }), GREEN);
	//DrawRay(Ray(Vector3{ 0,0,0 }, Vector3{ 0, 0, 1 }), BLUE);

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

RenderModel RenderModel::BuildFromShape(Cacti::Body body, Cacti::Shape* shape)
{
	static const Color colorList[] = {
	LIGHTGRAY, GRAY, DARKGRAY, YELLOW, GOLD, ORANGE, PINK, RED, MAROON,
	GREEN, LIME, DARKGREEN, SKYBLUE, BLUE, DARKBLUE, PURPLE, VIOLET,
	DARKPURPLE, BEIGE, BROWN, DARKBROWN, WHITE, BLACK, MAGENTA, RAYWHITE
	};
	static const size_t colorCount = sizeof(colorList) / sizeof(colorList[0]);

	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_int_distribution<size_t> dist(0, colorCount - 1);




	if (shape->GetType() == Cacti::Shape::ShapeType::SPHERE)
	{
		Cacti::Sphere* sphereShape = (Cacti::Sphere*)shape;
		Model sphere = LoadModelFromMesh(GenMeshSphere(sphereShape->radius, 50, 50));
		Vector3 raylibPos = { body.position.x, body.position.y, body.position.z };
		Color randomColor = colorList[dist(gen)];
		RenderModel sphereObj{ sphere, randomColor, raylibPos };


		return sphereObj;
	}

	else if (shape->GetType() == Cacti::Shape::CONVEX) {
		const Cacti::Convex* shapeConvex = (const Cacti::Convex*)shape;

		// Build the convex hull from the points
		std::vector<Vec3> hullPts;
		std::vector<Cacti::tri> hullTris;
		Cacti::BuildConvexHull(shapeConvex->points, hullPts, hullTris);

		// Calculate smoothed normals per vertex
		std::vector<Vec3> normals;
		normals.reserve(hullPts.size());
		for (int i = 0; i < (int)hullPts.size(); i++) {
			Vec3 norm(0.0f);
			for (int t = 0; t < (int)hullTris.size(); t++) {
				const Cacti::tri& tri = hullTris[t];
				if (i != tri.a && i != tri.b && i != tri.c)
					continue;
				const Vec3& a = hullPts[tri.a];
				const Vec3& b = hullPts[tri.b];
				const Vec3& c = hullPts[tri.c];
				Vec3 ab = b - a;
				Vec3 ac = c - a;
				norm += ab.Cross(ac);
			}
			norm.Normalize();
			normals.push_back(norm);
		}

		int vertCount = (int)hullPts.size();
		int indexCount = (int)hullTris.size() * 3;

		Mesh mesh = {};
		mesh.vertexCount = vertCount;
		mesh.triangleCount = (int)hullTris.size();

		mesh.vertices = (float*)RL_MALLOC(vertCount * 3 * sizeof(float));
		mesh.normals = (float*)RL_MALLOC(vertCount * 3 * sizeof(float));
		mesh.texcoords = (float*)RL_MALLOC(vertCount * 2 * sizeof(float));
		mesh.indices = (unsigned short*)RL_MALLOC(indexCount * sizeof(unsigned short));

		for (int i = 0; i < vertCount; i++) {
			mesh.vertices[i * 3 + 0] = hullPts[i].x;
			mesh.vertices[i * 3 + 1] = hullPts[i].y;
			mesh.vertices[i * 3 + 2] = hullPts[i].z;

			mesh.normals[i * 3 + 0] = normals[i].x;
			mesh.normals[i * 3 + 1] = normals[i].y;
			mesh.normals[i * 3 + 2] = normals[i].z;

			mesh.texcoords[i * 2 + 0] = 0.0f;
			mesh.texcoords[i * 2 + 1] = 0.0f;
		}

		for (int i = 0; i < (int)hullTris.size(); i++) {
			mesh.indices[i * 3 + 0] = (unsigned short)hullTris[i].a;
			mesh.indices[i * 3 + 1] = (unsigned short)hullTris[i].b;
			mesh.indices[i * 3 + 2] = (unsigned short)hullTris[i].c;
		}

		UploadMesh(&mesh, false);
		Model model = LoadModelFromMesh(mesh);
		Color randomColor = colorList[dist(gen)];
		Vector3 raylibPos = { body.position.x, body.position.y, body.position.z };
		return RenderModel(model, randomColor, raylibPos);
	}
	else if (shape->GetType() == Cacti::Shape::BOX)
	{
		const Cacti::Box* shapeBox = (const Cacti::Box*)shape;

		// use the physics box's actual 8 corner points directly
		// this guarantees the mesh matches the physics shape exactly
		Vec3 mn = shapeBox->bounds.mins;
		Vec3 mx = shapeBox->bounds.maxs;

		// 8 corners of the box in local space
		Vec3 corners[8] = {
			Vec3(mn.x, mn.y, mn.z),
			Vec3(mx.x, mn.y, mn.z),
			Vec3(mn.x, mx.y, mn.z),
			Vec3(mx.x, mx.y, mn.z),
			Vec3(mn.x, mn.y, mx.z),
			Vec3(mx.x, mn.y, mx.z),
			Vec3(mn.x, mx.y, mx.z),
			Vec3(mx.x, mx.y, mx.z),
		};

		// 6 faces, each as 2 triangles = 12 triangles = 36 indices
		// each face has 4 unique vertices for correct normals
		// 6 faces * 4 verts = 24 vertices total
		int vertCount = 24;
		int indexCount = 36;

		Mesh mesh = {};
		mesh.vertexCount = vertCount;
		mesh.triangleCount = 12;
		mesh.vertices = (float*)RL_MALLOC(vertCount * 3 * sizeof(float));
		mesh.normals = (float*)RL_MALLOC(vertCount * 3 * sizeof(float));
		mesh.texcoords = (float*)RL_MALLOC(vertCount * 2 * sizeof(float));
		mesh.indices = (unsigned short*)RL_MALLOC(indexCount * sizeof(unsigned short));

		// face definitions: each face = 4 corner indices + outward normal
		struct Face {
			int c[4];       // indices into corners[]
			Vec3 normal;
		};

		Face faces[6] = {
			{ {4,5,7,6}, Vec3(0, 0, 1) }, // front  +Z
			{ {0,2,3,1}, Vec3(0, 0,-1) }, // back   -Z
			{ {2,6,7,3}, Vec3(0, 1, 0) }, // top    +Y
			{ {0,1,5,4}, Vec3(0,-1, 0) }, // bottom -Y
			{ {1,3,7,5}, Vec3(1, 0, 0) }, // right  +
			{ {0,4,6,2}, Vec3(-1, 0, 0) }, // left   -X
		};

		

		for (int f = 0; f < 6; f++)
		{
			int baseVert = f * 4;

			for (int v = 0; v < 4; v++)
			{
				Vec3 p = corners[faces[f].c[v]];
				int idx = (baseVert + v) * 3;
				mesh.vertices[idx + 0] = p.x;
				mesh.vertices[idx + 1] = p.y;
				mesh.vertices[idx + 2] = p.z;

				mesh.normals[idx + 0] = faces[f].normal.x;
				mesh.normals[idx + 1] = faces[f].normal.y;
				mesh.normals[idx + 2] = faces[f].normal.z;

				int uvIdx = (baseVert + v) * 2;
				mesh.texcoords[uvIdx + 0] = (v == 1 || v == 3) ? 1.0f : 0.0f;
				mesh.texcoords[uvIdx + 1] = (v == 2 || v == 3) ? 1.0f : 0.0f;
			}

			// two triangles per face
			int baseIdx = f * 6;
			mesh.indices[baseIdx + 0] = baseVert + 0;
			mesh.indices[baseIdx + 1] = baseVert + 1;
			mesh.indices[baseIdx + 2] = baseVert + 2;
			mesh.indices[baseIdx + 3] = baseVert + 0;
			mesh.indices[baseIdx + 4] = baseVert + 2;
			mesh.indices[baseIdx + 5] = baseVert + 3;
		}

		UploadMesh(&mesh, false);
		Model model = LoadModelFromMesh(mesh);

		Color randomColor = colorList[dist(gen)];

		Vector3 raylibPos = { body.position.x, body.position.y, body.position.z };
		return RenderModel(model, randomColor, raylibPos);
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

