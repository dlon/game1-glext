#pragma once

#include <Python.h>
#include "batch.hpp"

class Texture;

extern PyTypeObject glrenderer_TextureRegionType;

class TextureRegion
{
protected:
	void writeVertices(std::vector<Batch::attributeType> &vertexAttribData, int offset, GLfloat x, GLfloat y);
	void writeTexCoords(std::vector<Batch::attributeType> &vertexAttribData, int offset, GLfloat x, GLfloat y);
	void writeColors(std::vector<Batch::attributeType> &vertexAttribData, int offset, GLfloat x, GLfloat y);
	
	void computeTransformations();
	GLfloat relativeVertices[8] = { 0 };

public:
	const Texture& texture;
	GLfloat tx, ty, tw, th;
	GLfloat width, height;
	GLfloat color[4] = { 1, 1, 1, 1 };
	GLfloat origin[2] = { 0 };
	GLfloat angle = 0;

	TextureRegion(const Texture& texture, int subX, int subY, int subWidth, int subHeight);
	virtual ~TextureRegion();

	float getOriginX() { return origin[0]; }
	float getOriginY() { return origin[1]; }

	float getAngle() { return angle; }
	void setAngle(float rads) { angle = rads; }
	
	float getScaleX() { return width / tw; }
	void setScaleX(float s) { width = s * tw; }
	float getScaleY() { return height / th; }
	void setScaleY(float s) { height = s * th; }
	
	void updateArray(std::vector<Batch::attributeType> &vertexAttribData, int objectIndex, GLfloat x, GLfloat y);
};

struct glrenderer_TextureRegion {
	PyObject_HEAD
	TextureRegion *_object;
};
