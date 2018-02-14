#pragma once

#include <Python.h>
#include "batch.hpp"

class Texture;

extern PyTypeObject glrenderer_TextureRegionType;

class TextureRegion
{
	void writeVertices(std::vector<Batch::attributeType> &vertexAttribData, int offset, GLfloat x, GLfloat y);
	void writeTexCoords(std::vector<Batch::attributeType> &vertexAttribData, int offset, GLfloat x, GLfloat y);
	void writeColors(std::vector<Batch::attributeType> &vertexAttribData, int offset, GLfloat x, GLfloat y);
public:
	const Texture& texture;
	GLfloat tx, ty, tw, th;
	GLfloat color[4] = { 1, 1, 1, 1 };

	TextureRegion(const Texture& texture, int subX, int subY, int subWidth, int subHeight);
	virtual ~TextureRegion();

	void setAngle(float rads);
	void setScaleX(float s);
	void setScaleY(float s);
	void transform(float angle, float sx, float sy);
	void updateArray(std::vector<Batch::attributeType> &vertexAttribData, int objectIndex, GLfloat x, GLfloat y);
};

struct glrenderer_TextureRegion {
	PyObject_HEAD
	TextureRegion *_object;
};
