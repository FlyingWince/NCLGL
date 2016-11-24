#include "Renderer.h"

Renderer::Renderer(Window & parent) : OGLRenderer(parent) {
	camera = new Camera();
	heightMap = new HeightMap("../../Textures/new.raw");
	quad = Mesh::GenerateQuad();
	camera->SetPosition(Vector3(RAW_WIDTH * HEIGHTMAP_X / 4.0f,
		3000.0f, RAW_WIDTH * HEIGHTMAP_X/2.0f));
	light = new Light(Vector3((RAW_HEIGHT * HEIGHTMAP_X / 4.0f * 3.0f), 300.0f,
		(RAW_HEIGHT * HEIGHTMAP_Z / 4.0f * 1.0f)),
		Vector4(0.9f, 0.9f, 1.0f, 1),
		(RAW_WIDTH * HEIGHTMAP_X) * 1.5f);
	triangle = Mesh::GenerateTriangle();
	//particle
	bubbles = new ParticleEmitter("../../Textures/33.png");
	test_emitter = new ParticleEmitter("grass.png");
	explosion1 = new ParticleEmitter("../../Textures/DarkRed2.JPG");
	explosion2 = new ParticleEmitter("../../Textures/DarkRed2.JPG");
	explosion3 = new ParticleEmitter("../../Textures/DarkRed2.JPG");
	explosion4 = new ParticleEmitter("../../Textures/DarkRed2.JPG");
	//control explo
	button = false;


	time = 0;
	fpscounter = 0;
	fps = 0;

	//hellData = new MD5FileData("../../Meshes/hellknight.md5mesh");
	//hellNode = new MD5Node(*hellData);
	//hellData->AddAnim("../../Meshes/idle2.md5anim");
	//hellNode->PlayAnim("../../Meshes/idle2.md5anim");

	reflectShader = new Shader("../../Shaders/PerPixelVertex.glsl",
		"../../Shaders/reflectFragment.glsl");
	skyboxShader = new Shader("../../Shaders/skyboxVertex.glsl",
		"../../Shaders/skyboxFragment.glsl");
	lightShader = new Shader("../../Shaders/CWPerPixelVertex.glsl",
		"../../Shaders/CWPerPixelFragment.glsl");
	sceneShader = new Shader("../../Shaders/ShadowSceneVertexC.glsl",
		"../../Shaders/ShadowSceneFragmentC.glsl");
	shadowShader = new Shader("../../Shaders/ShadowVertexC.glsl",
		"../../Shaders/ShadowFragmentC.glsl");
	triangleShader = new Shader("../../basicVertex.glsl", "../../colourFragment.glsl");
	//text
	textShader = new Shader(SHADERDIR"TexturedVertex.glsl", SHADERDIR"TexturedFragment.glsl");
	//particle
	particleShader = new Shader("../../Shaders/Vertex.glsl",	"../../Shaders/Fragment.glsl",	"../../Shaders/Geometry.glsl");
	if (!textShader->LinkProgram()) {
		return;
	}
	basicFont = new Font(SOIL_load_OGL_texture(TEXTUREDIR"tahoma.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_COMPRESS_TO_DXT), 16, 16);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	
	if (!particleShader->LinkProgram())
	{
		return;
	}
	/*if (!triangleShader->LinkProgram())
	{
		return;
	}*/

	if (!sceneShader->LinkProgram() || !shadowShader->LinkProgram()) {
		return;
	}
	if (!reflectShader->LinkProgram()) {
		return;
	}
	if (!lightShader->LinkProgram()) {
		return;
	}
	if (!skyboxShader->LinkProgram()) {
		return;
	}

	quad->SetTexture(SOIL_load_OGL_texture("../../Textures/watertexture2.tga",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	heightMap->SetTexture(SOIL_load_OGL_texture(
		"../../Textures/BarrenReds1.jpg", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	heightMap->SetBumpMap(SOIL_load_OGL_texture(
		"../../Textures/BarrenRedsDOT3.jpg", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));
	cubeMap = SOIL_load_OGL_cubemap(
		"../../Textures/skybox_west.jpg", "../../Textures/skybox_east.jpg",
		"../../Textures/skybox_up.jpg", "../../Textures/skybox_down.jpg",
		"../../Textures/skybox_north.jpg", "../../Textures/skybox_south.jpg",
		SOIL_LOAD_RGB,
		SOIL_CREATE_NEW_ID, 0
		);

	if (!cubeMap) {
		return;
	}
	if (!quad->GetTexture()) {
		return;
	}
	if (!heightMap->GetTexture()) {
		return;
	}
	if (!heightMap->GetBumpMap()) {
		return;
	}

	if (!explosion1)
	{
		return;
	}
	if (!explosion2)
	{
		return;
	}

	SetTextureRepeating(quad->GetTexture(), true);
	SetTextureRepeating(heightMap->GetTexture(), true);
	SetTextureRepeating(heightMap->GetBumpMap(), true);
	//from shadow
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
		GL_COMPARE_R_TO_TEXTURE);

	glBindTexture(GL_TEXTURE_2D, 0);
	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, shadowTex, 0);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/*floor = Mesh::GenerateQuad();
	floor->SetTexture(SOIL_load_OGL_texture("../../Textures/brick.tga"
	, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));
	floor->SetBumpMap(SOIL_load_OGL_texture("../../Textures/brickDOT3.tga"
	, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));*/
	//
	
	waterRotate = 0.0f;

	projMatrix = Matrix4::Perspective(1.0f, 100000.0f,
		(float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	init = true;
}

Renderer::~Renderer() {
	delete camera;
	delete heightMap;
	delete quad;
	delete reflectShader;
	delete skyboxShader;
	delete lightShader;
	delete light;
	delete triangle;
	//from shadow
	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);
	//delete hellData;
	//delete hellNode;
	/*delete floor;*/
	delete sceneShader;
	delete shadowShader;
	//
	currentShader = NULL;
	delete basicFont;
	//
	delete bubbles;
	delete test_emitter;
	delete explosion1;
	delete explosion2;
	delete explosion3;
	delete explosion4;
}

void Renderer::UpdateScene(float msec) {
	camera->UpdateCamera(msec);
	viewMatrix = camera->BuildViewMatrix();
	waterRotate += msec / 2000.0f;

	fpscounter += 1;
	Calculate(msec);


	//hellNode->Update(msec);
	bubbles->Update(msec);
	test_emitter->Update(msec);
	explosion1->Update(msec);
	explosion2->Update(msec);
	explosion3->Update(msec);
	explosion4->Update(msec);
}

void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	DrawSkybox();

	DrawShadowScene(); // First render pass ...
	DrawCombinedScene(); // Second render pass ...
					
	Memory();//if i do memory after text theres nothing can be seen , why?
	Text(fps);//and theres still some  api errors when run Nsight ?

	DrawParticle();
	
	/*if (Window::GetKeyboard()->KeyDown(KEYBOARD_E)) {
		button = !button;
	}*/
	if (button) {
		DrawExplosion();
	}
	//if (Window::GetKeyboard()->KeyDown(KEYBOARD_E)) {
	//	DrawExplosion();
	//}


	//2
	//if (t*explosionrate > 1) {
	//	DrawExplosion();
	//	t = t - 2.0f;
	//}
	//t = t + 1.0f;

	SwapBuffers();
}

void Renderer::DrawSkybox() {
	glDepthMask(GL_FALSE);
	SetCurrentShader(skyboxShader);
	UpdateShaderMatrices();
	quad->Draw();
	glUseProgram(0);
	glDepthMask(GL_TRUE);
}

void Renderer::DrawHeightmap() {
	SetCurrentShader(lightShader);
	SetShaderLight(*light);
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(),
		"cameraPos"), 1, (float *)& camera->GetPosition());
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"bumpTex"), 1);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"shadowTex"), 3);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, shadowTex);//bind shadowTex

	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	Matrix4 tempMatrix = shadowMatrix * modelMatrix;

	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram()
		, "shadowMatrix"), 1, false, *& tempMatrix.values);

	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram()
		, "modelMatrix"), 1, false, *& modelMatrix.values);

	//textureMatrix = Matrix4::BuildViewMatrix(
	//light->GetPosition(), Vector3(0, 0, 0));

	//shadowMatrix.ToIdentity();
	//shadowMatrix = Matrix4::BuildViewMatrix (light -> GetPosition(), Vector3(0, 0, 0));

	UpdateShaderMatrices();
	heightMap->Draw();
	glUseProgram(0);
}

void Renderer::DrawWater() {
	SetCurrentShader(reflectShader);
	SetShaderLight(*light);
	/*glEnable(GL_BLEND);*/
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(),
	"cameraPos"), 1, (float *)& camera->GetPosition());
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
	"diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"cubeTex"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	float heightX = (RAW_WIDTH * HEIGHTMAP_X * 4.0f);
	float heightY = 256 * HEIGHTMAP_Y / 3.0f + 50.f;
	float heightZ = (RAW_HEIGHT * HEIGHTMAP_Z * 4.0f);
	modelMatrix =
		Matrix4::Translation(Vector3(heightX, heightY, heightZ)) *
		Matrix4::Scale(Vector3(heightX, 1, heightZ)) *
		Matrix4::Rotation(90, Vector3(1.0f, 0.0f, 0.0f));
	textureMatrix = Matrix4::Scale(Vector3(10.0f, 10.0f, 10.0f)) *
		Matrix4::Rotation(waterRotate, Vector3(0.0f, 0.0f, 1.0f));
	//shadow
	Matrix4 tempMatrix = shadowMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram()
		, "shadowMatrix"), 1, false, *& tempMatrix.values);
	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram()
		, "modelMatrix"), 1, false, *& modelMatrix.values);

	UpdateShaderMatrices();

	////mirror repeat
	//glBindTexture(GL_TEXTURE_2D, quad->GetTexture());
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, //x axis
	//	true ? GL_MIRRORED_REPEAT : GL_CLAMP);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, //y axis
	//	true ? GL_MIRRORED_REPEAT : GL_CLAMP);
	//glBindTexture(GL_TEXTURE_2D, 0);

	quad->Draw();
	glUseProgram(0);
}

void Renderer::DrawShadowScene() {
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	SetCurrentShader(shadowShader);
	viewMatrix = Matrix4::BuildViewMatrix(
		light->GetPosition(), Vector3(0, 0, 0));

	shadowMatrix = biasMatrix *(projMatrix * viewMatrix);

	//textureMatrix = biasMatrix *(projMatrix * viewMatrix);
	UpdateShaderMatrices();
	//DrawFloor();

	DrawWater();
	DrawHeightmap();

	glUseProgram(0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawCombinedScene() {
	/*SetCurrentShader(sceneShader);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
	"diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
	"bumpTex"), 1);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
	"shadowTex"), 2);
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(),
	"cameraPos"), 1, (float *)& camera->GetPosition());
	SetShaderLight(*light);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);*/
	viewMatrix = camera->BuildViewMatrix();
	UpdateShaderMatrices();
	//DrawFloor();
	//DrawMesh();
	DrawWater();
	DrawHeightmap();
	/*glUseProgram(0);*/
}

void Renderer::DrawMesh() {
	modelMatrix.ToIdentity();
	modelMatrix = Matrix4::Translation(Vector3(128, 0, 128));
	UpdateShaderMatrices();
	Matrix4 tempMatrix = textureMatrix * modelMatrix;

	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram()
		, "textureMatrix"), 1, false, *& tempMatrix.values);
	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram()
		, "modelMatrix"), 1, false, *& modelMatrix.values);

	/*hellNode->Draw(*this);*/
}

void Renderer::EasyTriangle() {
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	SetCurrentShader(triangleShader);
	triangle->Draw();
	
	/*glClear(GL_COLOR_BUFFER_BIT);*/
	glUseProgram(0);
}

void Renderer::Memory() {
	SetCurrentShader(textShader);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	GLint total_mem_kb = 0;
	glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX,
		&total_mem_kb);

	GLint cur_avail_mem_kb = 0;
	glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX,
		&cur_avail_mem_kb);

	GLint memoryMB = (total_mem_kb - cur_avail_mem_kb) / 1024;
	DrawText(("The GPU memory now using is "+to_string(memoryMB)+"MB"),Vector3(0, height-16, 0), 16.0f);
	textureMatrix.ToIdentity();
	
	glDisable(GL_BLEND);
	glUseProgram(0);

}

void Renderer::Calculate(float msec) {
	time += msec;
	if (time >= 1000) {
		fps = fpscounter / time * 1000;
		time -= 1000;
		fpscounter = 0;
	}

	string totalmemory = "The total memory being used is 000";
}

void Renderer::Text(float frame) {
	SetCurrentShader(textShader);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);
	
	DrawText(("The fps is now "+to_string(frame)), Vector3(0, 0, 0), 16.0f);
	
	DrawText("Getting fucked by OpenGL T^T !!!", Vector3(0, 0, 100), 256.0f, true);

	glUseProgram(0);
};

void Renderer::DrawText(const std::string &text, const Vector3 &position, const float size, const bool perspective) {
	//Create a new temporary TextMesh, using our line of text and our font
	TextMesh* mesh = new TextMesh(text, *basicFont);

	//This just does simple matrix setup to render in either perspective or
	//orthographic mode, there's nothing here that's particularly tricky.
	if (perspective) {
		textureMatrix.ToIdentity();
		modelMatrix = Matrix4::Translation(position) * Matrix4::Scale(Vector3(size, size, 1));
		viewMatrix = camera->BuildViewMatrix();
		projMatrix = Matrix4::Perspective(1.0f, 100000.0f, (float)width / (float)height, 45.0f);
	}
	else {
		//In ortho mode, we subtract the y from the height, so that a height of 0
		//is at the top left of the screen, which is more intuitive
		//(for me anyway...)
		textureMatrix.ToIdentity();
		modelMatrix = Matrix4::Translation(Vector3(position.x, height - position.y, position.z)) * Matrix4::Scale(Vector3(size, size, 1));
		viewMatrix.ToIdentity();
		projMatrix = Matrix4::Orthographic(-1.0f, 1.0f, (float)width, 0.0f, (float)height, 0.0f);
	}
	//Either way, we update the matrices, and draw the mesh
	UpdateShaderMatrices();
	mesh->Draw();

	delete mesh; //Once it's drawn, we don't need it anymore!
	Matrix4::Perspective(1.0f, 50000.0f, (float)width / (float)height, 45.0f);
}

void Renderer::DrawParticle()
{
	/*glEnable(GL_BLEND);*/
	SetCurrentShader(particleShader);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);

	SetShaderParticleSize(bubbles->GetParticleSize());
	bubbles->SetParticleSize(150.0f);
	bubbles->SetParticleVariance(1.0f);
	bubbles->SetLaunchParticles(3);
	bubbles->SetParticleLifetime(18000.0f);
	bubbles->SetParticleSpeed(0.4f);

	
	modelMatrix = Matrix4::Translation(Vector3((RAW_WIDTH * HEIGHTMAP_X / 2.0f) - 450.0f,
		1000.0f,
		(RAW_WIDTH * HEIGHTMAP_X / 2.0f) + 225.0f));

	UpdateShaderMatrices();
	glDepthMask(GL_FALSE);
	bubbles->Draw();
	glDepthMask(GL_TRUE);//delate the edge of particle

						 //particle mo
	SetShaderParticleSize(test_emitter->GetParticleSize());
	test_emitter->SetParticleRate(199);
	test_emitter->SetParticleSize(50.0f);
	test_emitter->SetParticleVariance(0.9f);
	test_emitter->SetLaunchParticles(5);
	test_emitter->SetParticleLifetime(20000.0f);
	test_emitter->SetParticleSpeed(0.5);

	modelMatrix = Matrix4::Translation(Vector3((RAW_WIDTH * HEIGHTMAP_X / 2.0f)-450.0f,
		700.0f,
		(RAW_WIDTH * HEIGHTMAP_X / 2.0f)+225.0f));

	UpdateShaderMatrices();
	glDepthMask(GL_FALSE);
	test_emitter->Draw();
	glDepthMask(GL_TRUE);//delate the edge of particle

	/*glDisable(GL_BLEND);*/
}

void Renderer::DrawExplosion()
{

	SetShaderParticleSize(explosion1->GetParticleSize());
	explosion1->SetParticleRate(500);
	explosion1->SetParticleSize(200.0f);
	explosion1->SetParticleVariance(1.0f);
	explosion1->SetLaunchParticles(10);
	explosion1->SetParticleLifetime(10000.0f);
	explosion1->SetParticleSpeed(1.0f);

	modelMatrix = Matrix4::Translation(Vector3((RAW_WIDTH * HEIGHTMAP_X / 2.0f + 10000.0f),
		2600.0f,
		(RAW_WIDTH * HEIGHTMAP_X / 2.0f) + 2800.0f))	* Matrix4::Rotation(45 , Vector3(0,0,1));

	UpdateShaderMatrices();
	glDepthMask(GL_FALSE);

	explosion1->Draw();
	glDepthMask(GL_TRUE);//delate the edge of particle

	SetShaderParticleSize(explosion2->GetParticleSize());
	explosion2->SetParticleRate(500);
	explosion2->SetParticleSize(200.0f);
	explosion2->SetParticleVariance(1.0f);
	explosion2->SetLaunchParticles(10);
	explosion2->SetParticleLifetime(10000.0f);
	explosion2->SetParticleSpeed(1.0f);

	modelMatrix = Matrix4::Translation(Vector3((RAW_WIDTH * HEIGHTMAP_X / 2.0f + 10000.0f),
		2600.0f,
		(RAW_WIDTH * HEIGHTMAP_X / 2.0f) + 2800.0f))	* Matrix4::Rotation(-15 , Vector3(0,0,1));

	UpdateShaderMatrices();
	glDepthMask(GL_FALSE);

	explosion2->Draw();
	glDepthMask(GL_TRUE);//delate the edge of particle

	SetShaderParticleSize(explosion2->GetParticleSize());
	explosion3->SetParticleRate(500);
	explosion3->SetParticleSize(200.0f);
	explosion3->SetParticleVariance(1.0f);
	explosion3->SetLaunchParticles(10);
	explosion3->SetParticleLifetime(10000.0f);
	explosion3->SetParticleSpeed(1.0f);

	modelMatrix = Matrix4::Translation(Vector3((RAW_WIDTH * HEIGHTMAP_X / 2.0f + 10000.0f),
		2600.0f,
		(RAW_WIDTH * HEIGHTMAP_X / 2.0f) + 2800.0f))	* Matrix4::Rotation(15, Vector3(1, 0, 0));

	UpdateShaderMatrices();
	glDepthMask(GL_FALSE);

	explosion3->Draw();
	glDepthMask(GL_TRUE);//delate the edge of particle

	SetShaderParticleSize(explosion4->GetParticleSize());
	explosion4->SetParticleRate(500);
	explosion4->SetParticleSize(200.0f);
	explosion4->SetParticleVariance(1.0f);
	explosion4->SetLaunchParticles(10);
	explosion4->SetParticleLifetime(10000.0f);
	explosion4->SetParticleSpeed(1.0f);

	modelMatrix = Matrix4::Translation(Vector3((RAW_WIDTH * HEIGHTMAP_X / 2.0f + 10000.0f),
		2600.0f,
		(RAW_WIDTH * HEIGHTMAP_X / 2.0f) + 2800.0f))	* Matrix4::Rotation(-60, Vector3(0.5, 0, 0.5));

	UpdateShaderMatrices();
	glDepthMask(GL_FALSE);

	explosion4->Draw();
	glDepthMask(GL_TRUE);//delate the edge of particle


	/*glDisable(GL_BLEND);*/
}

void Renderer::SetShaderParticleSize(float f)
{
	glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "particleSize"), f);
}

//void Renderer::DrawFloor() {
//	modelMatrix = Matrix4::Rotation(90, Vector3(1, 0, 0)) *
//		Matrix4::Scale(Vector3(450, 450, 1));
//	Matrix4 tempMatrix = textureMatrix * modelMatrix;
//	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram()
//		, "textureMatrix"), 1, false, *& tempMatrix.values);
//	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram()
//		, "modelMatrix"), 1, false, *& modelMatrix.values);
//	floor->Draw();
//}