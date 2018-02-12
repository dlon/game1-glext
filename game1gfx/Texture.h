#pragma once

#include "glutil.hpp"

class Texture
{
public:
	GLuint texture;
	GLuint width;
	GLuint height;

	Texture(int width, int height, const unsigned char *data);
	virtual ~Texture();
};
