#pragma once

#include "glutil.hpp"
#include <vector>

class Texture;
class TextureRegion;

class Batch {
public:
	typedef GLfloat attributeType;
	typedef GLushort indexType;

private:
	void setupShaders();
	void createBuffers();

protected:
	GLuint program;
	GLuint vertexShader;
	GLuint fragmentShader;

	GLuint indexVbo;
	std::vector<indexType> indices;
	GLuint vertexVbo;
	std::vector<attributeType> vertexAttribData;

	size_t maxBatchSize;
	int objectIndex;
	GLuint currentTexture;

	GLenum blendSrc = 0, blendDest = 0;
	GLenum blendSwitchSrc = 0, blendSwitchDest = 0;

	GLfloat mMatrix[3][3] = {
		{ 1, 0, 0 },
		{ 0, 1, 0 },
		{ 0, 0, 1 }
	};

public:
	const static int positionAttribute = 0;
	const static int texCoordAttribute = 1;
	const static int colorAttribute = 2;
	
	const static int vpMatrixUniform = 0;
	const static int textureUniform = 1;
	const static int mMatrixUniform = 2;

	Batch(size_t maxBatchSize);
	virtual ~Batch();

	void setBlendMode(GLenum src, GLenum dest);
	void getBlendMode(GLenum ret[2]);

	void begin();
	void flush();
	void end();

	void draw(TextureRegion &textureRegion, float x, float y);

	void followCamera(float parallax, float x, float y);
	void ignoreCamera();

	size_t getBatchSize() { return maxBatchSize;  }
	GLuint getProgram() { return program; }
	int getObjectIndex() { return objectIndex; }
};
