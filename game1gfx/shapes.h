#pragma once

#include <Python.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include "glutil.hpp"
#include "GLProgram.h"


typedef struct {
	PyObject_HEAD
	
	GLProgram *program;
	GLuint programGL;
	GLProgram *circleProgram;

	GLuint vbo;
	GLuint vao;
	GLuint vaoCircle;
	float pointSize;
	unsigned int maxVertices;
	float rgba[4];
	int type;
	int vertCount;
	GLfloat *vertexData;
	int blendEnabled;
	GLenum blendMode[2];
	GLfloat mMatrix[3][3];
} glrenderer_ShapeBatch;

#define NO_PRIMITIVE -1
#define CIRCLE_RENDER -2

extern "C" {
	extern PyTypeObject ShapeBatch_type;
}

void updateData_nc(glrenderer_ShapeBatch *self, float x, float y);

template<typename ...Coord>
void updateData_nc(glrenderer_ShapeBatch *self, float x, float y, Coord ... coords)
{
	updateData_nc(self, x, y);
	updateData_nc(self, coords ...);
}

template<GLuint primitiveType, GLuint vertsNum>
void prepareShapeBatch(glrenderer_ShapeBatch *self)
{
	if (self->type != primitiveType ||
		self->vertCount + vertsNum > self->maxVertices) {
		ShapeBatch_end(self);
		self->type = primitiveType;
	}
}

#define SHAPEBATCH_DEFAULT_SMOOTHNESS 40

void ShapeBatch_begin(glrenderer_ShapeBatch *self);
void ShapeBatch_end(glrenderer_ShapeBatch *self);

void ShapeBatch_drawCircle(
	glrenderer_ShapeBatch *self,
	float x, float y,
	float radius
);
void ShapeBatch_drawCircle(
	glrenderer_ShapeBatch *self,
	float x, float y,
	float radius,
	size_t smoothness
);

template<typename ... Coordinate>
void ShapeBatch_drawPoints(glrenderer_ShapeBatch *self, Coordinate ... coords)
{
	prepareShapeBatch<GL_POINTS, sizeof...(coords) / 2>(self);
	updateData_nc(self, coords ...);
}

template<typename ... Coordinate>
void ShapeBatch_drawLines(glrenderer_ShapeBatch *self, Coordinate... coords)
{
	prepareShapeBatch<GL_LINES, sizeof...(coords) / 2>(self);
	updateData_nc(self, coords ...);
}

void ShapeBatch_setColor(glrenderer_ShapeBatch *self, float r, float g, float b, float a);
void ShapeBatch_setBlendMode(glrenderer_ShapeBatch *self, GLenum src, GLenum dest);
void ShapeBatch_getBlendMode(glrenderer_ShapeBatch *self, GLenum ret[2]);
void ShapeBatch_enableBlending(glrenderer_ShapeBatch *self);
void ShapeBatch_disableBlending(glrenderer_ShapeBatch *self);

void ShapeBatch_ignoreCamera(glrenderer_ShapeBatch *self);
