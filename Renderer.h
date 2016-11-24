# pragma once
# include "../../nclgl/OGLRenderer.h"
# include "../../nclgl/Camera.h"
# include "../../nclgl/HeightMap.h"
# include "../../nclgl/MD5Mesh.h"
#include "../../nclgl/MD5Node.h"
#include "../../nclgl/TextMesh.h"
#include "../../nclgl/ParticleEmitter.h"

# define SHADOWSIZE 2048
#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049

class Renderer : public OGLRenderer {
public:
	Renderer(Window & parent);
	virtual ~Renderer(void);
	virtual void RenderScene();
	virtual void UpdateScene(float msec);
	bool button;//to trigger explosion

protected:
	void DrawHeightmap();
	void DrawWater();
	void DrawSkybox();
	void DrawParticle();
	void SetShaderParticleSize(float f);
	void DrawMesh();
	/*void DrawFloor();*/
	void DrawShadowScene();
	void DrawCombinedScene();
	void DrawExplosion();


	Shader * lightShader;
	Shader * reflectShader;
	Shader * skyboxShader;
	//text
	Shader * textShader;

	Font*	basicFont;	//A font! a basic one...
						//
	void Calculate(float msec);
	void Text(float frame);
	void DrawText(const std::string &text, const Vector3 &position, const float size = 10.0f, const bool perspective = false);
	void Memory();

	void EasyTriangle();

	Shader * sceneShader;
	Shader * shadowShader;
	Shader * particleShader;
	Shader * triangleShader;
	GLuint shadowTex;
	GLuint shadowFBO;
	MD5FileData * hellData;
	MD5Node * hellNode;
	Mesh * floor;

	HeightMap * heightMap;
	Mesh * quad;

	Mesh*	triangle;

	Light * light;
	Camera * camera;
	GLuint cubeMap;
	float waterRotate;
	
	float  time;
	GLuint fpscounter;
	float fps;

	ParticleEmitter  * bubbles;
	ParticleEmitter  * test_emitter;
	ParticleEmitter  * explosion1;
	ParticleEmitter  * explosion2;
	ParticleEmitter  * explosion3;
	ParticleEmitter  * explosion4;

};