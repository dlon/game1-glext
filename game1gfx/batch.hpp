#pragma once

#include "glutil.hpp"

class Batch {

	void setupShaders();
	void createBuffers();

protected:
	GLuint program;
	GLuint vertexShader;
	GLuint fragmentShader;

	GLuint indexVbo;
	GLuint vertexVbo;

	size_t maxBatchSize;
	int objectIndex;
	GLuint currentTexture;

public:
	const static int positionAttribute = 0;
	const static int texCoordAttribute = 1;
	const static int colorAttribute = 2;
	const static int textureUniform = 1;

	Batch(size_t maxBatchSize);
	virtual ~Batch();
};
