#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include "texture.h"
#include "shaders.h"
#include "vertexBufferObject.h"

/********************************

Class:	CFreeTypeFont

Purpose: Wraps FreeType fonts and
		 their usage with OpenGL.

********************************/

class CFreeTypeFont
{
public:
	bool loadFont(string sFile, int iPXSize);
	bool loadSystemFont(string sName, int iPXSize);

	int getTextWidth(string sText, int iPXSize);

	void print(string sText, int x, int y, int iPXSize = -1);

	void releaseFont();

	void setShaderProgram(CShaderProgram* a_shShaderProgram);

	CFreeTypeFont();
private:
	void createChar(int iIndex);

	CTexture tCharTextures[256];
	int iAdvX[256], iAdvY[256];
	int iBearingX[256], iBearingY[256];
	int iCharWidth[256], iCharHeight[256];
	int iLoadedPixelSize, iNewLine;

	bool bLoaded;

	UINT uiVAO;
	CVertexBufferObject vboData;

	FT_Library ftLib;
	FT_Face ftFace;
	CShaderProgram* shShaderProgram;
};
