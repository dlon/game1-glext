#pragma once

#include "batch.hpp"

class TextureRegion
{
	void writeVertices(std::vector<Batch::attributeType> &vertexAttribData, int offset, GLfloat x, GLfloat y);
	void writeTexCoords(std::vector<Batch::attributeType> &vertexAttribData, int offset, GLfloat x, GLfloat y);
	void writeColors(std::vector<Batch::attributeType> &vertexAttribData, int offset, GLfloat x, GLfloat y);
public:
	const Texture& texture;
	GLfloat tx, ty, tw, th;

	TextureRegion(const Texture& texture, int subX, int subY, int subWidth, int subHeight);
	virtual ~TextureRegion();

	void setAngle(float rads);
	void setScaleX(float s);
	void setScaleY(float s);
	void transform(float angle, float sx, float sy);
	void updateArray(std::vector<Batch::attributeType> &vertexAttribData, int offset, GLfloat x, GLfloat y);
};

