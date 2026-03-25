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

Renderer::Renderer()
	:cam(),shader()
{
}

TextureCubemap Renderer::GenTextureCubemap(Shader shader, Texture2D panorama, int size, int format)
{
	TextureCubemap cubemap = { 0 };

	rlDisableBackfaceCulling();     // Disable backface culling to render inside the cube

	// STEP 1: Setup framebuffer
	//------------------------------------------------------------------------------------------
	unsigned int rbo = rlLoadTextureDepth(size, size, true);
	cubemap.id = rlLoadTextureCubemap(0, size, format);

	unsigned int fbo = rlLoadFramebuffer(size, size);
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

void Renderer::Init()
{
	shader = LoadShader(TextFormat("resources/shaders/glsl%i/lighting.vs", GLSL_VERSION),
		TextFormat("resources/shaders/glsl%i/lighting.fs", GLSL_VERSION));

	shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

	int ambientLoc = GetShaderLocation(shader, "ambient");
	float ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	SetShaderValue(shader, ambientLoc, ambient, SHADER_UNIFORM_VEC4);

	// Create lights ONCE here, not in Draw()
	lights.emplace_back(CreateLight(LIGHT_POINT, { -2, 10, -2 }, Vector3Zero(), WHITE, shader));


	Model cubeModel = LoadModelFromMesh(GenMeshCube(2.0f, 4.0f, 2.0f));
	Model planeModel = LoadModelFromMesh(GenMeshPlane(1000.0f, 1000.0f, 1, 1));

	cubeModel.materials[0].shader = shader;
	planeModel.materials[0].shader = shader;
	
	RenderModel cube(cubeModel, BLUE);
	RenderModel plane(planeModel, WHITE);


	sceneObjects.emplace_back(cube);
	sceneObjects.emplace_back(plane);

	Mesh skyboxMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
	skyboxModel = LoadModelFromMesh(skyboxMesh);

	// Load the skybox display shader
	skyboxModel.materials[0].shader = LoadShader(
		TextFormat("resources/shaders/glsl%i/skybox.vs", GLSL_VERSION),
		TextFormat("resources/shaders/glsl%i/skybox.fs", GLSL_VERSION));

	// Tell the skybox shader which texture slot is the cubemap
	int envMap = MATERIAL_MAP_CUBEMAP;
	SetShaderValue(skyboxModel.materials[0].shader,
		GetShaderLocation(skyboxModel.materials[0].shader, "environmentMap"),
		&envMap, SHADER_UNIFORM_INT);

	// Load the cubemap conversion shader (panorama -> cubemap)
	Shader shdrCubemap = LoadShader(
		TextFormat("resources/shaders/glsl%i/cubemap.vs", GLSL_VERSION),
		TextFormat("resources/shaders/glsl%i/cubemap.fs", GLSL_VERSION));

	int equirect = 0;
	SetShaderValue(shdrCubemap,
		GetShaderLocation(shdrCubemap, "equirectangularMap"),
		&equirect, SHADER_UNIFORM_INT);

	// Load HDR panorama and convert it to a cubemap
	Texture2D panorama = LoadTexture("resources/skybox.png");
	skyboxModel.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture =
		GenTextureCubemap(shdrCubemap, panorama, 1024, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

	// These are no longer needed after cubemap generation
	UnloadTexture(panorama);
	UnloadShader(shdrCubemap);

}


void Renderer::Update()
{
	cam.Update();

	Camera3D camera = cam.ReturnCam();
	float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };

	SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

	for (auto& light : lights) UpdateLightValues(shader, light);

	BeginDrawing();
		ClearBackground(BLACK);
		BeginMode3D(camera);

			rlDisableBackfaceCulling();
			rlDisableDepthMask();
			DrawModel(skyboxModel, Vector3Zero(), 1.0f, WHITE);
			rlEnableBackfaceCulling();
			rlEnableDepthMask();
			
			for (RenderModel& obj : sceneObjects)
			{
				obj.Draw();
			}
	
		EndMode3D();
	EndDrawing();
}

void Renderer::Destroy()
{
	//UnloadTexture(textures.at("cube_texture"));

	for (RenderModel& obj : sceneObjects)
	{
		UnloadModel(obj.model);
	}

	UnloadShader(skyboxModel.materials[0].shader);
	UnloadTexture(skyboxModel.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture);
	UnloadModel(skyboxModel);
}

RenderModel::RenderModel(Model& model, Color color)
{
	this->model = model;
	this->color = color;
}

void RenderModel::Draw()
{
	DrawModelEx(this->model, Vector3Zero(), Vector3Zero(), 0.0f, Vector3One(), this->color);
}
