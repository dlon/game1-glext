#include <Python.h>
#include <structmember.h>
#include "glutil.hpp"
#include <math.h>
#include "shapes.h"

#define PI 3.14159265f

extern "C" float surfaceWidth;
extern "C" float surfaceHeight;

/*
shapeVertexPositionAttribute = 0
shapeVertexColorAttribute = 1

shapeVMatrixUniform = 0
shapeMMatrixUniform = 1
shapePointSizeUniform = 2
*/

static const char vertexShaderSource[] =
"#version 440\n"

"layout(location = 0) uniform mat3 vpMatrix;"
"layout(location = 1) uniform mat3 mMatrix;"
"layout(location = 2) uniform float pointSize;"

"layout(location = 0) in vec2 vPosition;"
"layout(location = 1) in vec4 vColor;"

"out vec4 vertColor;"

"void main(void) {"
"	gl_PointSize = pointSize;"
"	gl_Position = vec4(vpMatrix * mMatrix * vec3(vPosition, 1.0), 1.0);"
"	vertColor = vColor;"
"};";

static const char fragmentShaderSource[] =
"#version 440\n"

"in vec4 vertColor;"
"layout (location = 0) out vec4 colorResult;"

"void main(void) {"
"	colorResult = vertColor;"
"}";

void setUpShaders(glrenderer_ShapeBatch *self)
{
	// FIXME: add error handling/checking (compilation, linking (etc.?))
	const char* vshaders[] = { vertexShaderSource };
	const char* fshaders[] = { fragmentShaderSource };

	self->vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(
		self->vertexShader,
		1,
		vshaders,
		NULL
	);
	glCompileShader(self->vertexShader);

	self->fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(
		self->fragmentShader,
		1,
		fshaders,
		NULL
	);
	glCompileShader(self->fragmentShader);

	// TODO: error handling
	GLint isCompiledV = 0;
	GLint isCompiledF = 0;
	glGetShaderiv(self->vertexShader, GL_COMPILE_STATUS, &isCompiledV);
	glGetShaderiv(self->fragmentShader, GL_COMPILE_STATUS, &isCompiledF);
	//printf("vertex shader: %d\n", isCompiledV);
	//printf("fragment shader: %d\n", isCompiledF);
	if (!isCompiledV)
		printShaderInfoLog(self->vertexShader);
	if (!isCompiledF)
		printShaderInfoLog(self->fragmentShader);

	self->program = glCreateProgram();

	glAttachShader(self->program, self->vertexShader);
	glAttachShader(self->program, self->fragmentShader);
	glLinkProgram(self->program);

	// TODO: error handling
	GLint isLinked = 0;
	glGetProgramiv(
		self->program,
		GL_LINK_STATUS,
		&isLinked
	);
	//printf("link status: %d\n", isLinked);
	if (!isLinked)
		printProgramInfoLog(self->program);

	glEnableVertexAttribArray(0); // vertex position attrib
	glEnableVertexAttribArray(1); // vertex color attrib

	GLfloat projectionMatrix[3][3] = {
		{ 2.0f / surfaceWidth, 0, 0 },
		{ 0, -2.0f / surfaceHeight, 0 },
		{ 0, 0, 1 }
	};
	GLfloat viewMatrix[3][3] = {
		{ 1, 0, 0 },
		{ 0, 1, 0 },
		{ -160, -120, 1 }
	};

	GLfloat matrix[3][3] = { 0 };
	for (int i = 0; i<3; i++) {
		for (int j = 0; j<3; j++) {
			for (int k = 0; k<3; k++) {
				matrix[i][j] += viewMatrix[i][j] * projectionMatrix[k][j];
			}
		}
	}

	self->mMatrix[0][0] = 1;
	self->mMatrix[0][1] = 0;
	self->mMatrix[0][2] = 0;
	self->mMatrix[1][0] = 0;
	self->mMatrix[1][1] = 1;
	self->mMatrix[1][2] = 0;
	self->mMatrix[2][0] = 0;
	self->mMatrix[2][1] = 0;
	self->mMatrix[2][2] = 1;

	glUseProgram(self->program);
	glUniformMatrix3fv(
		0, // vp matrix
		1,
		GL_FALSE,
		(GLfloat*)matrix
	);
	glUniformMatrix3fv(
		1, // m matrix
		1,
		GL_FALSE,
		(GLfloat*)self->mMatrix
	);

	glEnable(GL_PROGRAM_POINT_SIZE);
}

/*****

C interface

*****/

void ShapeBatch_begin(glrenderer_ShapeBatch *self)
{
	glUseProgram(self->program);
}

void ShapeBatch_end(glrenderer_ShapeBatch *self)
{
	if (!self->type)
		return;

	glEnable(GL_BLEND);
	glBlendFunc(self->blendMode[0], self->blendMode[1]);

	glBindVertexArray(self->vao);

	glBindBuffer(GL_ARRAY_BUFFER, self->vbo);
	glBufferData(
		GL_ARRAY_BUFFER,
		6 * self->vertCount * sizeof(GLfloat),
		self->vertexData,
		GL_DYNAMIC_DRAW // TODO: try using *Sub*
	);

	glDrawArrays(
		self->type,
		0,
		self->vertCount
	);

	glBindVertexArray(0);

	self->vertCount = 0;
	self->type = 0;
}

void ShapeBatch_drawCircle(
	glrenderer_ShapeBatch *self,
	float x, float y,
	float radius,
	size_t smoothness
) {
	// TODO: use pure triangles instead of a fan (allows batching)?
	if (self->type != GL_TRIANGLE_FAN) {
		ShapeBatch_end(self);
		self->type = GL_TRIANGLE_FAN;
	}

	if (self->vertCount + smoothness > self->maxVertices)
		ShapeBatch_end(self);

	size_t pCount = self->vertCount + smoothness;
	size_t ppCount = self->vertCount;

	float r = PyFloat_AsDouble(PyTuple_GetItem(self->color, 0));
	float g = PyFloat_AsDouble(PyTuple_GetItem(self->color, 1));
	float b = PyFloat_AsDouble(PyTuple_GetItem(self->color, 2));
	float a = PyFloat_AsDouble(PyTuple_GetItem(self->color, 3));

	if (PyErr_Occurred()) {
		// FIXME: handle
		return;
	}

	for (Py_ssize_t i = ppCount; i < pCount; i++)
	{
		self->vertexData[6 * i + 0] =
			x + radius * cos((float)(i - ppCount) / smoothness * 2.0f * PI);
		self->vertexData[6 * i + 1] =
			y + radius * sin((float)(i - ppCount) / smoothness * 2.0f * PI);
		self->vertexData[6 * i + 2] = r;
		self->vertexData[6 * i + 3] = g;
		self->vertexData[6 * i + 4] = b;
		self->vertexData[6 * i + 5] = a;
	}

	self->vertCount += smoothness;

	ShapeBatch_end(self);
}

void ShapeBatch_drawCircle(
	glrenderer_ShapeBatch *self,
	float x, float y,
	float radius
) {
	ShapeBatch_drawCircle(self, x, y, radius, SHAPEBATCH_DEFAULT_SMOOTHNESS);
}

void ShapeBatch_setColor(glrenderer_ShapeBatch *self,
	float r, float g, float b, float a)
{
	PyTuple_SET_ITEM(self->color, 0, PyFloat_FromDouble(r));
	PyTuple_SET_ITEM(self->color, 1, PyFloat_FromDouble(g));
	PyTuple_SET_ITEM(self->color, 2, PyFloat_FromDouble(b));
	PyTuple_SET_ITEM(self->color, 3, PyFloat_FromDouble(a));
}

void ShapeBatch_setBlendMode(glrenderer_ShapeBatch *self, GLenum src, GLenum dest)
{
	if (src != self->blendMode[0] || dest != self->blendMode[1])
	{
		ShapeBatch_end(self);
		self->blendMode[0] = src;
		self->blendMode[1] = dest;
	}
}

void ShapeBatch_getBlendMode(glrenderer_ShapeBatch *self, GLenum ret[2])
{
	ret[0] = self->blendMode[0];
	ret[1] = self->blendMode[1];
}

/*****

Python interface

*****/

static PyObject* glrenderer_ShapeBatch_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	glrenderer_ShapeBatch *self = (glrenderer_ShapeBatch*)type->tp_alloc(type, 0);
	if (self) {
		float r, g, b, a;
		r = 1.0f;
		g = 1.0f;
		b = 1.0f;
		a = 1.0f;
		self->color = Py_BuildValue("(ffff)", r, g, b, a);
		if (!self->color) {
			Py_DECREF(self);
			return NULL;
		}
		self->pointSize = 1.0f;
	}
	return (PyObject*)self;
}

static void glrenderer_ShapeBatch_dealloc(glrenderer_ShapeBatch *self)
{
	glDeleteShader(self->fragmentShader);
	glDeleteShader(self->vertexShader);
	glDeleteProgram(self->program);

	glDeleteBuffers(1, &self->vbo);
	glDeleteVertexArrays(1, &self->vao);

	free(self->vertexData);
	Py_XDECREF(self->color);
	Py_TYPE(self)->tp_free(self);
}

static int glrenderer_ShapeBatch_init(glrenderer_ShapeBatch *self, PyObject *args, PyObject *kwds)
{
	static char *kwlist[] = { "maxVertices", 0 };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "I", kwlist,
		&self->maxVertices))
		return -1;

	if (self->maxVertices == 0)
		return -1;

	self->vertexData = (GLfloat*) malloc(6 * sizeof(GLfloat) * self->maxVertices);
	if (!self->vertexData) {
		return -1;
	}
	memset(self->vertexData, 0, 6 * sizeof(GLfloat) * self->maxVertices);

	setUpShaders(self);

	glGenBuffers(1, &self->vbo);
	glGenVertexArrays(1, &self->vao);

	glBindBuffer(GL_ARRAY_BUFFER, self->vbo);
	glBufferData(
		GL_ARRAY_BUFFER,
		6 * self->maxVertices,
		self->vertexData,
		GL_DYNAMIC_DRAW
	);

	glBindVertexArray(self->vao);
	glEnableVertexAttribArray(0); // vertex position attrib
	glEnableVertexAttribArray(1); // vertex color attrib
	glVertexAttribPointer(
		0, // position
		2,
		GL_FLOAT,
		GL_FALSE,
		6 * sizeof(GLfloat),
		0 * sizeof(GLfloat)
	);
	glVertexAttribPointer(
		1, // color
		4,
		GL_FLOAT,
		GL_TRUE,
		6 * sizeof(GLfloat),
		(const GLvoid*)(2 * sizeof(GLfloat))
	);
	glBindVertexArray(0);

	self->blendMode[0] = GL_SRC_ALPHA;
	self->blendMode[1] = GL_ONE_MINUS_SRC_ALPHA;

	return 0;
}

static PyObject *glrenderer_ShapeBatch_begin(glrenderer_ShapeBatch *self)
{
	ShapeBatch_begin(self);
	Py_RETURN_NONE;
}

static PyObject *glrenderer_ShapeBatch_end(glrenderer_ShapeBatch *self)
{
	ShapeBatch_end(self);
	Py_RETURN_NONE;
}

static void updateData(glrenderer_ShapeBatch *self, PyObject *args)
{
	float x, y;
	Py_ssize_t verts = PyTuple_GET_SIZE(args);
	Py_ssize_t pointsNum = verts / 2;

	if (self->vertCount + pointsNum > self->maxVertices)
		ShapeBatch_end(self);

	size_t pCount = self->vertCount + pointsNum;
	size_t ppCount = self->vertCount;

	/*float r = PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(self->color, 0));
	float g = PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(self->color, 1));
	float b = PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(self->color, 2));
	float a = PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(self->color, 3));*/
	// TODO: convert using setter (if worthwhile)
	float r = PyFloat_AsDouble(PyTuple_GET_ITEM(self->color, 0));
	float g = PyFloat_AsDouble(PyTuple_GET_ITEM(self->color, 1));
	float b = PyFloat_AsDouble(PyTuple_GET_ITEM(self->color, 2));
	float a = PyFloat_AsDouble(PyTuple_GET_ITEM(self->color, 3));

	for (Py_ssize_t i = ppCount; i < pCount; i++)
	{
		//x = PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(args, 2 * (i - ppCount) + 0));
		//y = PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(args, 2 * (i - ppCount) + 1));
		// TODO: never check?
		x = PyFloat_AsDouble(PyTuple_GET_ITEM(args, 2 * (i - ppCount) + 0));
		y = PyFloat_AsDouble(PyTuple_GET_ITEM(args, 2 * (i - ppCount) + 1));
		self->vertexData[6 * i + 0] = x;
		self->vertexData[6 * i + 1] = y;
		self->vertexData[6 * i + 2] = r;
		self->vertexData[6 * i + 3] = g;
		self->vertexData[6 * i + 4] = b;
		self->vertexData[6 * i + 5] = a;
	}

	self->vertCount += pointsNum;
}

static PyObject *glrenderer_ShapeBatch_lineStrip(glrenderer_ShapeBatch *self, PyObject *args)
{
	if (self->type != GL_LINE_STRIP) {
		ShapeBatch_end(self);
		self->type = GL_LINE_STRIP;
	}
	updateData(self, args);
	ShapeBatch_end(self);
	Py_RETURN_NONE;
}

static PyObject *glrenderer_ShapeBatch_lines(glrenderer_ShapeBatch *self, PyObject *args)
{
	if (self->type != GL_LINES) {
		ShapeBatch_end(self);
		self->type = GL_LINES;
	}
	updateData(self, args);
	Py_RETURN_NONE;
}

static PyObject *glrenderer_ShapeBatch_triangles(glrenderer_ShapeBatch *self, PyObject *args)
{
	if (self->type != GL_TRIANGLES) {
		ShapeBatch_end(self);
		self->type = GL_TRIANGLES;
	}
	updateData(self, args);
	Py_RETURN_NONE;
}

static PyObject *glrenderer_ShapeBatch_triangleFan(glrenderer_ShapeBatch *self, PyObject *args)
{
	if (self->type != GL_TRIANGLE_FAN) {
		ShapeBatch_end(self);
		self->type = GL_TRIANGLE_FAN;
	}
	updateData(self, args);
	ShapeBatch_end(self);
	Py_RETURN_NONE;
}

static PyObject *glrenderer_ShapeBatch_triangleStrip(glrenderer_ShapeBatch *self, PyObject *args)
{
	if (self->type != GL_TRIANGLE_STRIP) {
		ShapeBatch_end(self);
		self->type = GL_TRIANGLE_STRIP;
	}
	updateData(self, args);
	ShapeBatch_end(self);
	Py_RETURN_NONE;
}

static PyObject *glrenderer_ShapeBatch_points(glrenderer_ShapeBatch *self, PyObject *args)
{
	if (self->type != GL_POINTS) {
		ShapeBatch_end(self);
		self->type = GL_POINTS;
	}
	glUniform1f(2, self->pointSize);
	updateData(self, args);
	Py_RETURN_NONE;
}

static PyObject *glrenderer_ShapeBatch_rectangle(glrenderer_ShapeBatch *self, PyObject *args)
{
	// could use strip but it wouldn't be batchable
	if (self->type != GL_TRIANGLES) {
		ShapeBatch_end(self);
		self->type = GL_TRIANGLES;
	}

	float x = PyFloat_AsDouble(PyTuple_GET_ITEM(args, 0));
	float y = PyFloat_AsDouble(PyTuple_GET_ITEM(args, 1));
	float w = PyFloat_AsDouble(PyTuple_GET_ITEM(args, 2));
	float h = PyFloat_AsDouble(PyTuple_GET_ITEM(args, 3));

	size_t i = 6 * self->vertCount;

	float r = PyFloat_AsDouble(PyTuple_GET_ITEM(self->color, 0));
	float g = PyFloat_AsDouble(PyTuple_GET_ITEM(self->color, 1));
	float b = PyFloat_AsDouble(PyTuple_GET_ITEM(self->color, 2));
	float a = PyFloat_AsDouble(PyTuple_GET_ITEM(self->color, 3));

	self->vertexData[i + 6 * 0 + 0] = x;
	self->vertexData[i + 6 * 0 + 1] = y;
	self->vertexData[i + 6 * 1 + 0] = x;
	self->vertexData[i + 6 * 1 + 1] = y + h;
	self->vertexData[i + 6 * 2 + 0] = x + w;
	self->vertexData[i + 6 * 2 + 1] = y;
	self->vertexData[i + 6 * 3 + 0] = x + w;
	self->vertexData[i + 6 * 3 + 1] = y;
	self->vertexData[i + 6 * 4 + 0] = x;
	self->vertexData[i + 6 * 4 + 1] = y + h;
	self->vertexData[i + 6 * 5 + 0] = x + w;
	self->vertexData[i + 6 * 5 + 1] = y + h;

	for (int j = 0; j < 6; j++) {
		self->vertexData[i + 6 * j + 2] = r;
		self->vertexData[i + 6 * j + 3] = g;
		self->vertexData[i + 6 * j + 4] = b;
		self->vertexData[i + 6 * j + 5] = a;
	}

	self->vertCount += 6;
	Py_RETURN_NONE;
}

static PyObject *glrenderer_ShapeBatch_circle(glrenderer_ShapeBatch *self, PyObject *args)
{
	float x, y, radius;
	size_t smoothness = SHAPEBATCH_DEFAULT_SMOOTHNESS;
	if (!PyArg_ParseTuple(args, "fff|I",
		&x, &y, &radius, &smoothness))
		return NULL;

	ShapeBatch_drawCircle(self, x, y, radius, smoothness);
	Py_RETURN_NONE;
}

static PyObject *
glrenderer_ShapeBatch_getBlendMode(glrenderer_ShapeBatch *self, void *closure)
{
	return Py_BuildValue("II", self->blendMode[0], self->blendMode[1]);
}

static int
glrenderer_ShapeBatch_setBlendMode(glrenderer_ShapeBatch *self, PyObject *args, void *closure)
{
	GLenum src, dest;
	if (!PyArg_ParseTuple(args, "II", &src, &dest))
		return -1;
	ShapeBatch_setBlendMode(self, src, dest);
	return 0;
}

static PyObject *
glrenderer_ShapeBatch_getColor255(glrenderer_ShapeBatch *self, void *closure)
{
	return Py_BuildValue("ffff",
		255.0f * PyFloat_AsDouble(PyTuple_GET_ITEM(self->color, 0)),
		255.0f * PyFloat_AsDouble(PyTuple_GET_ITEM(self->color, 1)),
		255.0f * PyFloat_AsDouble(PyTuple_GET_ITEM(self->color, 2)),
		255.0f * PyFloat_AsDouble(PyTuple_GET_ITEM(self->color, 3)));
}

static int
glrenderer_ShapeBatch_setColor255(glrenderer_ShapeBatch *self, PyObject *args, void *closure)
{
	float r, g, b, a;
	if (!PyArg_ParseTuple(args, "ffff",
		&r, &g, &b, &a))
		return -1;
	PyTuple_SET_ITEM(self->color, 0, PyFloat_FromDouble(r / 255.0f));
	PyTuple_SET_ITEM(self->color, 1, PyFloat_FromDouble(g / 255.0f));
	PyTuple_SET_ITEM(self->color, 2, PyFloat_FromDouble(b / 255.0f));
	PyTuple_SET_ITEM(self->color, 3, PyFloat_FromDouble(a / 255.0f));
	return 0;
}

static PyObject *
glrenderer_ShapeBatch_get3Color255(glrenderer_ShapeBatch *self, void *closure)
{
	float r = PyFloat_AsDouble(PyTuple_GetItem(self->color, 0));
	float g = PyFloat_AsDouble(PyTuple_GetItem(self->color, 1));
	float b = PyFloat_AsDouble(PyTuple_GetItem(self->color, 2));
	return Py_BuildValue("fff",
		255.0f * r,
		255.0f * g,
		255.0f * b
	);
}

static int
glrenderer_ShapeBatch_set3Color255(glrenderer_ShapeBatch *self, PyObject *args, void *closure)
{
	float r, g, b;
	if (!PyArg_ParseTuple(args, "fff",
		&r, &g, &b))
		return -1;
	PyTuple_SetItem(self->color, 0, PyFloat_FromDouble(r / 255.0f));
	PyTuple_SetItem(self->color, 1, PyFloat_FromDouble(g / 255.0f));
	PyTuple_SetItem(self->color, 2, PyFloat_FromDouble(b / 255.0f));
	return 0;
}

static PyObject *
glrenderer_ShapeBatch_getAlpha(glrenderer_ShapeBatch *self, void *closure)
{
	PyObject *obj = PyTuple_GetItem(self->color, 3);
	if (!obj) {
		return NULL;
	}
	Py_INCREF(obj);
	return obj;
}

static int
glrenderer_ShapeBatch_setAlpha(glrenderer_ShapeBatch *self, PyObject *args, void *closure)
{
	Py_INCREF(args);
	PyTuple_SET_ITEM(self->color, 3, args);
	return 0;
}

static int
glrenderer_ShapeBatch_setColor(glrenderer_ShapeBatch *self, PyObject *args, void *closure)
{
	float r, g, b, a;
	if (!PyArg_ParseTuple(args, "ffff",
		&r, &g, &b, &a))
		return -1;
	PyTuple_SET_ITEM(self->color, 0, PyFloat_FromDouble(r));
	PyTuple_SET_ITEM(self->color, 1, PyFloat_FromDouble(g));
	PyTuple_SET_ITEM(self->color, 2, PyFloat_FromDouble(b));
	PyTuple_SET_ITEM(self->color, 3, PyFloat_FromDouble(a));
	return 0;
}

static PyObject *
glrenderer_ShapeBatch_getColor(glrenderer_ShapeBatch *self, void *closure)
{
	Py_INCREF(self->color);
	return self->color;
}

static PyObject *glrenderer_ShapeBatch_followCamera(glrenderer_ShapeBatch *self, PyObject *args)
{
	float parallax;
	float x, y;
	if (!PyArg_ParseTuple(args, "fff",
		&parallax, &x, &y))
		return NULL;

	self->mMatrix[2][0] = -x * parallax;
	self->mMatrix[2][1] = -y * parallax;

	glUseProgram(self->program);
	glUniformMatrix3fv(
		1, // model uniform
		1,
		GL_FALSE,
		(GLfloat*)self->mMatrix
	);

	Py_RETURN_NONE;
}

static PyObject *glrenderer_ShapeBatch_ignoreCamera(glrenderer_ShapeBatch *self, PyObject *args)
{
	float x = self->mMatrix[2][0];
	float y = self->mMatrix[2][1];

	self->mMatrix[2][0] = 0;
	self->mMatrix[2][1] = 0;

	glUseProgram(self->program);
	glUniformMatrix3fv(
		1, // model uniform
		1,
		GL_FALSE,
		(GLfloat*)self->mMatrix
	);

	self->mMatrix[2][0] = x;
	self->mMatrix[2][1] = y;

	Py_RETURN_NONE;
}

static PyMethodDef glrenderer_ShapeBatch_methods[] = {
	{ "begin", (PyCFunction)glrenderer_ShapeBatch_begin, METH_NOARGS, 0 },
	{ "end", (PyCFunction)glrenderer_ShapeBatch_end, METH_NOARGS, 0 },
	{ "drawLineStrip", (PyCFunction)glrenderer_ShapeBatch_lineStrip, METH_VARARGS, 0 },
	{ "drawLines", (PyCFunction)glrenderer_ShapeBatch_lines, METH_VARARGS, 0 },
	{ "drawTriangles", (PyCFunction)glrenderer_ShapeBatch_triangles, METH_VARARGS, 0 },
	{ "drawTriangleStrip", (PyCFunction)glrenderer_ShapeBatch_triangleStrip, METH_VARARGS, 0 },
	{ "drawTriangleFan", (PyCFunction)glrenderer_ShapeBatch_triangleFan, METH_VARARGS, 0 },
	{ "drawPoints", (PyCFunction)glrenderer_ShapeBatch_points, METH_VARARGS, 0 },
	{ "drawCircle", (PyCFunction)glrenderer_ShapeBatch_circle, METH_VARARGS, 0 },
	{ "drawRectangle", (PyCFunction)glrenderer_ShapeBatch_rectangle, METH_VARARGS, 0 },
	{ "followCamera", (PyCFunction)glrenderer_ShapeBatch_followCamera, METH_VARARGS, 0 },
	{ "ignoreCamera", (PyCFunction)glrenderer_ShapeBatch_ignoreCamera, METH_NOARGS, 0 },
	{ 0 }
};

static PyMemberDef glrenderer_ShapeBatch_members[] = {
	{ "program", T_UINT, offsetof(glrenderer_ShapeBatch, program), READONLY, 0 },
	{ "pointSize", T_FLOAT, offsetof(glrenderer_ShapeBatch, pointSize), 0, 0 },
	{ 0 }
};

static PyGetSetDef glrenderer_ShapeBatch_getset[] = {
	{ "blendMode", (getter)glrenderer_ShapeBatch_getBlendMode, (setter)glrenderer_ShapeBatch_setBlendMode, 0, 0 },
	{ "color255", (getter)glrenderer_ShapeBatch_getColor255, (setter)glrenderer_ShapeBatch_setColor255, 0, 0 },
	{ "color", (getter)glrenderer_ShapeBatch_getColor, (setter)glrenderer_ShapeBatch_setColor, 0, 0 },
	{ "rgb255", (getter)glrenderer_ShapeBatch_get3Color255, (setter)glrenderer_ShapeBatch_set3Color255, 0, 0 },
	{ "alpha", (getter)glrenderer_ShapeBatch_getAlpha, (setter)glrenderer_ShapeBatch_setAlpha, 0, 0 },
	{ 0 }
};

PyTypeObject ShapeBatch_type = {
	PyObject_HEAD_INIT(NULL, 0)
	"glrenderer.ShapeBatch",        /*tp_name*/
	sizeof(glrenderer_ShapeBatch),  /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)glrenderer_ShapeBatch_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	NULL, /*tp_doc*/
	0,
	0,
	0,
	0,
	0,
	0,
	glrenderer_ShapeBatch_methods,
	glrenderer_ShapeBatch_members,
	glrenderer_ShapeBatch_getset,
	0,
	0,
	0,
	0,
	0,
	(initproc)glrenderer_ShapeBatch_init,
	0,
	glrenderer_ShapeBatch_new
};
