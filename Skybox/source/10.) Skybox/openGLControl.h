#pragma once

#define SIMPLE_OPENGL_CLASS_NAME "simple_openGL_class_name"

/********************************

Class:	COpenGLControl

Purpose:	Provides convenient usage
			of OpenGL

********************************/

class COpenGLControl
{
public:
	bool initOpenGL(HINSTANCE hInstance, HWND* a_hWnd, int iMajorVersion, int iMinorVersion, void (*a_initScene)(LPVOID), void (*a_renderScene)(LPVOID), void(*a_releaseScene)(LPVOID), LPVOID lpParam);
	
	void resizeOpenGLViewportFull();
	void setProjection3D(float fFOV, float fAspectRatio, float fNear, float fFar);
	void setOrtho2D(int width, int height);

	glm::mat4* getProjectionMatrix();
	glm::mat4* getOrthoMatrix();

	void render(LPVOID lpParam);
	void releaseOpenGLControl(LPVOID lpParam);

	static void registerSimpleOpenGLClass(HINSTANCE hInstance);
	static void unregisterSimpleOpenGLClass(HINSTANCE hInstance);

	void makeCurrent();
	void swapBuffers();

	bool setVerticalSynchronization(bool bEnabled);

	int getFPS();

	int getViewportWidth();
	int getViewportHeight();

	COpenGLControl();

private:
	bool initGLEW(HINSTANCE hInstance);

	HDC hDC;
	HWND* hWnd;
	HGLRC hRC;
	static bool bClassRegistered;
	static bool bGlewInitialized;
	int iMajorVersion, iMinorVersion;

	// Used for FPS calculation
	int iFPSCount, iCurrentFPS;
	clock_t tLastSecond;

	// Matrix for perspective projection
	glm::mat4 mProjection;
	// Matrix for orthographic 2D projection
	glm::mat4 mOrtho;

	// Viewport parameters
	int iViewportWidth, iViewportHeight;

	void (*initScene)(LPVOID lpParam), (*renderScene)(LPVOID lpParam), (*releaseScene)(LPVOID lpParam);
};

LRESULT CALLBACK msgHandlerSimpleOpenGLClass(HWND, UINT, WPARAM, LPARAM);