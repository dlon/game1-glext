#pragma once

typedef struct {
	PyObject_HEAD
	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint program;
	GLuint vbo;
	GLuint vao;
	float pointSize;
	unsigned int maxVertices;
	float rgba[4];
	int type;
	int vertCount;
	GLfloat *vertexData;
	GLenum blendMode[2];
	GLfloat mMatrix[3][3];
} glrenderer_ShapeBatch;

extern "C" {
	extern PyTypeObject ShapeBatch_type;
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

void ShapeBatch_drawLines(glrenderer_ShapeBatch *self, int numVerts, ...);

void ShapeBatch_setColor(glrenderer_ShapeBatch *self, float r, float g, float b, float a);
void ShapeBatch_setBlendMode(glrenderer_ShapeBatch *self, GLenum src, GLenum dest);
void ShapeBatch_getBlendMode(glrenderer_ShapeBatch *self, GLenum ret[2]);

void ShapeBatch_ignoreCamera(glrenderer_ShapeBatch *self);
