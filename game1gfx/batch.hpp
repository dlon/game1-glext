#pragma once

#include "glutil.hpp"
#include <vector>
#include "Texture.h"

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

public:
	const static int positionAttribute = 0;
	const static int texCoordAttribute = 1;
	const static int colorAttribute = 2;
	
	const static int vpMatrixUniform = 0;
	const static int textureUniform = 1;

	Batch(size_t maxBatchSize);
	virtual ~Batch();

	void begin();
	void flush();
	void end();

	void draw(const Texture &texture); // test
};
