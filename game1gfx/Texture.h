#pragma once

#include "glutil.hpp"

extern PyTypeObject glrenderer_TextureType;

class Texture
{
public:
	GLuint texture;
	GLuint width;
	GLuint height;

	void loadData(int width, int height, const unsigned char *data);

	Texture() {}
	Texture(int width, int height, const unsigned char *data);
	virtual ~Texture();
};
