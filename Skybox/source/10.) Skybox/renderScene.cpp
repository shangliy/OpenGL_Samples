#include "common_header.h"

#include "win_OpenGLApp.h"

#include "shaders.h"
#include "texture.h"
#include "vertexBufferObject.h"

#include "flyingCamera.h"

#include "freeTypeFont.h"

#include "skybox.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/* One VBO, where all static data are stored now,
in this tutorial vertex is stored as 3 floats for
position, 2 floats for texture coordinate and
3 floats for normal vector. */

CVertexBufferObject vboSceneObjects;
UINT uiVAOs[1]; // Only one VAO now

CShader shShaders[5];
CShaderProgram spDirectionalLight, spOrtho2D, spFont2D;

#define NUMTEXTURES 3

CTexture tTextures[NUMTEXTURES];

CFlyingCamera cCamera;

CFreeTypeFont ftFont;

CSkybox sbMainSkybox;

/*-----------------------------------------------

Name:		initScene

Params:	lpParam - Pointer to anything you want.

Result:	Initializes OpenGL features that will
			be used.

/*---------------------------------------------*/

#include "static_geometry.h"

int iTorusFaces;

void initScene(LPVOID lpParam)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	vboSceneObjects.createVBO();
	glGenVertexArrays(1, uiVAOs); // Create one VAO
	glBindVertexArray(uiVAOs[0]);

	vboSceneObjects.bindVBO();

	// Add cube to VBO

	FOR(i, 36)
	{
		vboSceneObjects.addData(&vCubeVertices[i], sizeof(glm::vec3));
		vboSceneObjects.addData(&vCubeTexCoords[i%6], sizeof(glm::vec2));
		vboSceneObjects.addData(&vCubeNormals[i/6], sizeof(glm::vec3));
	}

	// Add ground to VBO

	FOR(i, 6)
	{
		vboSceneObjects.addData(&vGround[i], sizeof(glm::vec3));
		vCubeTexCoords[i] *= 10.0f;
		vboSceneObjects.addData(&vCubeTexCoords[i%6], sizeof(glm::vec2));
		glm::vec3 vGroundNormal(0.0f, 1.0f, 0.0f);
		vboSceneObjects.addData(&vGroundNormal, sizeof(glm::vec3));
	}

	iTorusFaces = generateTorus(vboSceneObjects, 7.0f, 2.0f, 20, 20);
	vboSceneObjects.uploadDataToGPU(GL_STATIC_DRAW);

	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3)+sizeof(glm::vec2), 0);
	// Texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3)+sizeof(glm::vec2), (void*)sizeof(glm::vec3));
	// Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3)+sizeof(glm::vec2), (void*)(sizeof(glm::vec3)+sizeof(glm::vec2)));

	// Load shaders and create shader programs

	shShaders[0].loadShader("data\\shaders\\shader.vert", GL_VERTEX_SHADER);
	shShaders[1].loadShader("data\\shaders\\shader.frag", GL_FRAGMENT_SHADER);
	shShaders[2].loadShader("data\\shaders\\ortho2D.vert", GL_VERTEX_SHADER);
	shShaders[3].loadShader("data\\shaders\\ortho2D.frag", GL_FRAGMENT_SHADER);
	shShaders[4].loadShader("data\\shaders\\font2D.frag", GL_FRAGMENT_SHADER);

	spDirectionalLight.createProgram();
	spDirectionalLight.addShaderToProgram(&shShaders[0]);
	spDirectionalLight.addShaderToProgram(&shShaders[1]);
	spDirectionalLight.linkProgram();

	spOrtho2D.createProgram();
	spOrtho2D.addShaderToProgram(&shShaders[2]);
	spOrtho2D.addShaderToProgram(&shShaders[3]);
	spOrtho2D.linkProgram();

	spFont2D.createProgram();
	spFont2D.addShaderToProgram(&shShaders[2]);
	spFont2D.addShaderToProgram(&shShaders[4]);
	spFont2D.linkProgram();

	// Load textures

	string sTextureNames[] = {"grass.jpg", "crate.jpg", "metalplate.jpg"};

	FOR(i, NUMTEXTURES) // I know that FOR cycle is useless now, but it was easier to rewrite :)
	{
		tTextures[i].loadTexture2D("data\\textures\\"+sTextureNames[i], true);
		tTextures[i].setFiltering(TEXTURE_FILTER_MAG_BILINEAR, TEXTURE_FILTER_MIN_BILINEAR_MIPMAP);
	}

	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);
	glClearColor(0.0f, 0.26f, 0.48f, 1.0f);

	// Here we load font with pixel size 32 - this means that if we print with size above 32, the quality will be low
	ftFont.loadSystemFont("arial.ttf", 32);
	ftFont.setShaderProgram(&spFont2D);
	
	cCamera = CFlyingCamera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), 25.0f, 0.1f);
	cCamera.setMovingKeys('W', 'S', 'A', 'D');

	sbMainSkybox.loadSkybox("data\\skyboxes\\jajlands1\\", "jajlands1_ft.jpg", "jajlands1_bk.jpg", "jajlands1_lf.jpg", "jajlands1_rt.jpg", "jajlands1_up.jpg", "jajlands1_dn.jpg");
}

/*-----------------------------------------------

Name:	renderScene

Params:	lpParam - Pointer to anything you want.

Result:	Renders whole scene.

/*---------------------------------------------*/

float fGlobalAngle;

void renderScene(LPVOID lpParam)
{
	// Typecast lpParam to COpenGLControl pointer
	COpenGLControl* oglControl = (COpenGLControl*)lpParam;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_TEXTURE_2D);
	spDirectionalLight.useProgram();

	spDirectionalLight.setUniform("sunLight.vColor", glm::vec3(1.0f, 1.0f, 1.0f));
	spDirectionalLight.setUniform("sunLight.fAmbientIntensity", 1.0f); // Full light for skybox
	spDirectionalLight.setUniform("sunLight.vDirection", glm::vec3(0, -1, 0));

	spDirectionalLight.setUniform("projectionMatrix", oglControl->getProjectionMatrix());
	spDirectionalLight.setUniform("gSampler", 0);

	glm::mat4 mModelView = cCamera.look();
	glm::mat4 mModelToCamera;

	spDirectionalLight.setUniform("modelViewMatrix", glm::translate(mModelView, cCamera.vEye));
	sbMainSkybox.renderSkybox();

	glBindVertexArray(uiVAOs[0]);
	spDirectionalLight.setUniform("sunLight.fAmbientIntensity", 0.55f);
	spDirectionalLight.setUniform("modelViewMatrix", &mModelView);
	
	// Render ground

	tTextures[0].bindTexture();
	glDrawArrays(GL_TRIANGLES, 36, 6);

	tTextures[1].bindTexture();

	SFOR(nb, 1, 9)
	{
		int iCnt = nb > 5 ? 10-nb : nb;
		FOR(i, iCnt)
		{
			glm::vec3 vPos = glm::vec3(-20.0f+nb*8.02f, -6.0f+i*8.02f, -50.0f);
			mModelToCamera = glm::translate(glm::mat4(1.0), vPos);
			mModelToCamera = glm::scale(mModelToCamera, glm::vec3(8.0f, 8.0f, 8.0f));
			// We need to transform normals properly, it's done by transpose of inverse matrix of rotations and scales
			spDirectionalLight.setUniform("normalMatrix", glm::transpose(glm::inverse(mModelToCamera)));
			spDirectionalLight.setUniform("modelViewMatrix", mModelView*mModelToCamera);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	}

	// Render 3 rotated tori to create interesting object

	tTextures[2].bindTexture();
	spDirectionalLight.setUniform("vColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	
	glm::vec3 vPos = glm::vec3(0.0f, 0.0, 40.0f);
	mModelToCamera = glm::translate(glm::mat4(1.0), vPos);
	mModelToCamera = glm::rotate(mModelToCamera, fGlobalAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	spDirectionalLight.setUniform("normalMatrix", glm::transpose(glm::inverse(mModelToCamera)));
	spDirectionalLight.setUniform("modelViewMatrix", mModelView*mModelToCamera);
	glDrawArrays(GL_TRIANGLES, 42, iTorusFaces*3);

	mModelToCamera = glm::translate(glm::mat4(1.0), vPos);
	mModelToCamera = glm::rotate(mModelToCamera, fGlobalAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	mModelToCamera = glm::rotate(mModelToCamera, 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	spDirectionalLight.setUniform("normalMatrix", glm::transpose(glm::inverse(mModelToCamera)));
	spDirectionalLight.setUniform("modelViewMatrix", mModelView*mModelToCamera);
	glDrawArrays(GL_TRIANGLES, 42, iTorusFaces*3);

	mModelToCamera = glm::translate(glm::mat4(1.0), vPos);
	mModelToCamera = glm::rotate(mModelToCamera, fGlobalAngle+90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	spDirectionalLight.setUniform("normalMatrix", glm::transpose(glm::inverse(mModelToCamera)));
	spDirectionalLight.setUniform("modelViewMatrix", mModelView*mModelToCamera);
	glDrawArrays(GL_TRIANGLES, 42, iTorusFaces*3);


	fGlobalAngle += appMain.sof(100.0f);
	cCamera.update();

	// Print something over scene

	spFont2D.useProgram();
	glDisable(GL_DEPTH_TEST);
	spFont2D.setUniform("projectionMatrix", oglControl->getOrthoMatrix());
	spFont2D.setUniform("vColor", glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));

	ftFont.print("www.mbsoftworks.sk", 20, 20);

	glEnable(GL_DEPTH_TEST);
	if(Keys::onekey(VK_ESCAPE))PostQuitMessage(0);

	oglControl->swapBuffers();
}

/*-----------------------------------------------

Name:	releaseScene

Params:	lpParam - Pointer to anything you want.

Result:	Releases OpenGL scene.

/*---------------------------------------------*/

void releaseScene(LPVOID lpParam)
{
	FOR(i, NUMTEXTURES)tTextures[i].releaseTexture();
	sbMainSkybox.releaseSkybox();

	spDirectionalLight.deleteProgram();
	spOrtho2D.deleteProgram();
	spFont2D.deleteProgram();
	FOR(i, 4)shShaders[i].deleteShader();
	ftFont.releaseFont();

	glDeleteVertexArrays(1, uiVAOs);
	vboSceneObjects.releaseVBO();
}