#pragma once

#include "glutil.hpp"

class Batch {

	void setupShaders();

protected:
	GLuint vertexShader;
	GLuint fragmentShader;

public:
	const static int positionAttribute = 0;
	const static int texCoordAttribute = 1;
	const static int colorAttribute = 2;

	Batch();
	virtual ~Batch();
};
