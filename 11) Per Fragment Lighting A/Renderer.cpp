#include "Renderer.h"
Renderer::Renderer(Window & parent) : OGLRenderer(parent) {
	camera = new Camera(0.0f, 0.0f, Vector3(
	RAW_WIDTH * HEIGHTMAP_X / 2.0f, 500, RAW_HEIGHT * HEIGHTMAP_Z));
	heightMap = new HeightMap("../../Textures/terrain.raw");
	currentShader = new Shader("../../Shaders/PerPixelVertex.glsl",
		"../../Shaders/PerPixelFragment.glsl");
	heightMap->SetTexture(SOIL_load_OGL_texture(
		"../../Textures/BarrenReds.JPG", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));
	if (!currentShader->LinkProgram() || !heightMap->GetTexture()) {
		return;
	}

	SetTextureRepeating(heightMap->GetTexture(), true);
	for (int i = 0;i < 2;i++) {
		for (int j = 0;j < 2;j++) {
			light[0 + i * 2 + j] = new Light(Vector3((RAW_HEIGHT * HEIGHTMAP_X / 4.0f * (1 + 2 * i)), 250.0f, (RAW_HEIGHT * HEIGHTMAP_Z / 4.0f *  (1 + 2 * j))),
				Vector4(0.7f*i , 0.7f*j, 0.7f*(i+j), 1), (RAW_WIDTH * HEIGHTMAP_X) / 4.0f);
		}
	}

	//Light light1(Vector3((RAW_HEIGHT * HEIGHTMAP_X / 4.0f*1),
	//	250.0f, (RAW_HEIGHT * HEIGHTMAP_Z / 4.0f*1)),
	//	Vector4(0, 0, 1, 1), (RAW_WIDTH * HEIGHTMAP_X) / 4.0f);
	//Light light2(Vector3((RAW_HEIGHT * HEIGHTMAP_X / 4.0f*1),
	//	250.0f, (RAW_HEIGHT * HEIGHTMAP_Z / 4.0f*3)),
	//	Vector4(0, 1, 0, 1), (RAW_WIDTH * HEIGHTMAP_X) / 4.0f);
	//Light light3(Vector3((RAW_HEIGHT * HEIGHTMAP_X / 4.0f*3),
	//	250.0f, (RAW_HEIGHT * HEIGHTMAP_Z / 4.0f*1)),
	//	Vector4(1, 0, 0, 1), (RAW_WIDTH * HEIGHTMAP_X) / 4.0f);
	//Light light4(Vector3((RAW_HEIGHT * HEIGHTMAP_X / 4.0f*3),
	//	250.0f, (RAW_HEIGHT * HEIGHTMAP_Z / 4.0f*3)),
	//	Vector4(0.7, 0.7, 0, 1), (RAW_WIDTH * HEIGHTMAP_X) / 4.0f);
	
	//use vector of pointer to Light could initialize by loops
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
	(float)width / (float)height, 45.0f);
	glEnable(GL_DEPTH_TEST);
	init = true;
}
Renderer ::~Renderer(void) {
	delete camera;
	delete heightMap;
	/*delete light;*/
}

void Renderer::UpdateScene(float msec) {
	camera->UpdateCamera(msec);
	viewMatrix = camera->BuildViewMatrix();
}

void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glUseProgram(currentShader->GetProgram());
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(),
		"diffuseTex"), 0);
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(),
		"cameraPos"), 1, (float *)& camera->GetPosition());
	UpdateShaderMatrices();
	Set4ShaderLight(*light);
	heightMap->Draw();
	glUseProgram(0);
	SwapBuffers();
}