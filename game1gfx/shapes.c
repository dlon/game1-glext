#include <Python.h>
#include <structmember.h>
#include "glutil.hpp"
#include <math.h>

#define PI 3.14159265

extern float surfaceWidth;
extern float surfaceHeight;

typedef struct {
	PyObject_HEAD
	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint program;
	GLuint vbo;
	GLuint vao;
	float pointSize;
	unsigned int maxVertices;
	PyObject *color;
	int type;
	int vertCount;
	GLfloat *vertexData;
	GLenum blendMode[2];
	GLfloat mMatrix[3][3];
} ShapeBatch;

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

void setUpShaders(ShapeBatch *self)
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

static PyObject* ShapeBatch_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	ShapeBatch *self = type->tp_alloc(type, 0);
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
	return self;
}

static void ShapeBatch_dealloc(ShapeBatch *self)
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

static int ShapeBatch_init(ShapeBatch *self, PyObject *args, PyObject *kwds)
{
	static char *kwlist[] = { "maxVertices", 0 };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "I", kwlist,
		&self->maxVertices))
		return -1;

	if (self->maxVertices == 0)
		return -1;

	self->vertexData = malloc(6 * sizeof(GLfloat) * self->maxVertices);
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
		2 * sizeof(GLfloat)
	);
	glBindVertexArray(0);

	self->blendMode[0] = GL_SRC_ALPHA;
	self->blendMode[1] = GL_ONE_MINUS_SRC_ALPHA;

	return 0;
}

static PyObject *ShapeBatch_begin(ShapeBatch *self)
{
	glUseProgram(self->program);
	Py_RETURN_NONE;
}

static void end(ShapeBatch *self)
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

static PyObject *ShapeBatch_end(ShapeBatch *self)
{
	end(self);
	Py_RETURN_NONE;
}

static void updateData(ShapeBatch *self, PyObject *args)
{
	float x, y;
	Py_ssize_t verts = PyTuple_GET_SIZE(args);
	Py_ssize_t pointsNum = verts / 2;

	if (self->vertCount + pointsNum > self->maxVertices)
		end(self);

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

static PyObject *ShapeBatch_lineStrip(ShapeBatch *self, PyObject *args)
{
	if (self->type != GL_LINE_STRIP) {
		end(self);
		self->type = GL_LINE_STRIP;
	}
	updateData(self, args);
	end(self);
	Py_RETURN_NONE;
}

static PyObject *ShapeBatch_lines(ShapeBatch *self, PyObject *args)
{
	if (self->type != GL_LINES) {
		end(self);
		self->type = GL_LINES;
	}
	updateData(self, args);
	Py_RETURN_NONE;
}

static PyObject *ShapeBatch_triangles(ShapeBatch *self, PyObject *args)
{
	if (self->type != GL_TRIANGLES) {
		end(self);
		self->type = GL_TRIANGLES;
	}
	updateData(self, args);
	Py_RETURN_NONE;
}

static PyObject *ShapeBatch_triangleFan(ShapeBatch *self, PyObject *args)
{
	if (self->type != GL_TRIANGLE_FAN) {
		end(self);
		self->type = GL_TRIANGLE_FAN;
	}
	updateData(self, args);
	end(self);
	Py_RETURN_NONE;
}

static PyObject *ShapeBatch_triangleStrip(ShapeBatch *self, PyObject *args)
{
	if (self->type != GL_TRIANGLE_STRIP) {
		end(self);
		self->type = GL_TRIANGLE_STRIP;
	}
	updateData(self, args);
	end(self);
	Py_RETURN_NONE;
}

static PyObject *ShapeBatch_points(ShapeBatch *self, PyObject *args)
{
	if (self->type != GL_POINTS) {
		end(self);
		self->type = GL_POINTS;
	}
	glUniform1f(2, self->pointSize);
	updateData(self, args);
	Py_RETURN_NONE;
}

static PyObject *ShapeBatch_rectangle(ShapeBatch *self, PyObject *args)
{
	// could use strip but it wouldn't be batchable
	if (self->type != GL_TRIANGLES) {
		end(self);
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

static PyObject *ShapeBatch_circle(ShapeBatch *self, PyObject *args)
{
	// TODO: use pure triangles instead of a fan (allows batching)?
	if (self->type != GL_TRIANGLE_FAN) {
		end(self);
		self->type = GL_TRIANGLE_FAN;
	}
	
	float x, y, radius;
	size_t smoothness = 40;
	if (!PyArg_ParseTuple(args, "fff|I",
		&x, &y, &radius, &smoothness))
		return NULL;

	if (self->vertCount + smoothness > self->maxVertices)
		end(self);

	size_t pCount = self->vertCount + smoothness;
	size_t ppCount = self->vertCount;

	/*float r = PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(self->color, 0));
	float g = PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(self->color, 1));
	float b = PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(self->color, 2));
	float a = PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(self->color, 3));*/
	// TODO: convert using setter (if worthwhile)
	/*float r = PyFloat_AsDouble(PyTuple_GET_ITEM(self->color, 0));
	float g = PyFloat_AsDouble(PyTuple_GET_ITEM(self->color, 1));
	float b = PyFloat_AsDouble(PyTuple_GET_ITEM(self->color, 2));
	float a = PyFloat_AsDouble(PyTuple_GET_ITEM(self->color, 3));*/
	float r = PyFloat_AsDouble(PyTuple_GetItem(self->color, 0));
	float g = PyFloat_AsDouble(PyTuple_GetItem(self->color, 1));
	float b = PyFloat_AsDouble(PyTuple_GetItem(self->color, 2));
	float a = PyFloat_AsDouble(PyTuple_GetItem(self->color, 3));

	if (PyErr_Occurred()) {
		return NULL;
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

	end(self);

	Py_RETURN_NONE;
}

static PyObject *
ShapeBatch_getBlendMode(ShapeBatch *self, void *closure)
{
	return Py_BuildValue("II", self->blendMode[0], self->blendMode[1]);
}

static int
ShapeBatch_setBlendMode(ShapeBatch *self, PyObject *args, void *closure)
{
	GLenum newBlendMode[2];
	if (!PyArg_ParseTuple(args, "II", &newBlendMode[0], &newBlendMode[1]))
		return -1;
	if (newBlendMode[0] != self->blendMode[0] || newBlendMode[1] != self->blendMode[1])
	{
		end(self);
		self->blendMode[0] = newBlendMode[0];
		self->blendMode[1] = newBlendMode[1];
	}
	return 0;
}

static PyObject *
ShapeBatch_getColor255(ShapeBatch *self, void *closure)
{
	return Py_BuildValue("ffff",
		255.0f * PyFloat_AsDouble(PyTuple_GET_ITEM(self->color, 0)),
		255.0f * PyFloat_AsDouble(PyTuple_GET_ITEM(self->color, 1)),
		255.0f * PyFloat_AsDouble(PyTuple_GET_ITEM(self->color, 2)),
		255.0f * PyFloat_AsDouble(PyTuple_GET_ITEM(self->color, 3)));
}

static int
ShapeBatch_setColor255(ShapeBatch *self, PyObject *args, void *closure)
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
ShapeBatch_get3Color255(ShapeBatch *self, void *closure)
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
ShapeBatch_set3Color255(ShapeBatch *self, PyObject *args, void *closure)
{
	float r, g, b;
	if (!PyArg_ParseTuple(args, "fff",
		&r, &g, &b))
		return -1;
	PyObject *prevR = PyTuple_GET_ITEM(self->color, 0);
	PyObject *prevG = PyTuple_GET_ITEM(self->color, 1);
	PyObject *prevB = PyTuple_GET_ITEM(self->color, 2);
	PyTuple_SET_ITEM(self->color, 0, PyFloat_FromDouble(r / 255.0f));
	PyTuple_SET_ITEM(self->color, 1, PyFloat_FromDouble(g / 255.0f));
	PyTuple_SET_ITEM(self->color, 2, PyFloat_FromDouble(b / 255.0f));
	return 0;
}

static PyObject *
ShapeBatch_getAlpha(ShapeBatch *self, void *closure)
{
	PyObject *obj = PyTuple_GetItem(self->color, 3);
	if (!obj) {
		return NULL;
	}
	Py_INCREF(obj);
	return obj;
}

static int
ShapeBatch_setAlpha(ShapeBatch *self, PyObject *args, void *closure)
{
	Py_INCREF(args);
	PyTuple_SET_ITEM(self->color, 3, args);
	return 0;
}

static int
ShapeBatch_setColor(ShapeBatch *self, PyObject *args, void *closure)
{
	float r, g, b, a;
	if (!PyArg_ParseTuple(args, "ffff",
		&r, &g, &b, &a))
		return -1;
	PyObject *prevR = PyTuple_GET_ITEM(self->color, 0);
	PyObject *prevG = PyTuple_GET_ITEM(self->color, 1);
	PyObject *prevB = PyTuple_GET_ITEM(self->color, 2);
	PyObject *prevA = PyTuple_GET_ITEM(self->color, 3);
	PyTuple_SET_ITEM(self->color, 0, PyFloat_FromDouble(r));
	PyTuple_SET_ITEM(self->color, 1, PyFloat_FromDouble(g));
	PyTuple_SET_ITEM(self->color, 2, PyFloat_FromDouble(b));
	PyTuple_SET_ITEM(self->color, 3, PyFloat_FromDouble(a));
	return 0;
}

static PyObject *
ShapeBatch_getColor(ShapeBatch *self, void *closure)
{
	Py_INCREF(self->color);
	return self->color;
}

static PyObject *ShapeBatch_followCamera(ShapeBatch *self, PyObject *args)
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

static PyObject *ShapeBatch_ignoreCamera(ShapeBatch *self, PyObject *args)
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

static PyMethodDef ShapeBatch_methods[] = {
	{ "begin", (PyCFunction)ShapeBatch_begin, METH_NOARGS, 0 },
	{ "end", (PyCFunction)ShapeBatch_end, METH_NOARGS, 0 },
	{ "drawLineStrip", ShapeBatch_lineStrip, METH_VARARGS, 0 },
	{ "drawLines", ShapeBatch_lines, METH_VARARGS, 0 },
	{ "drawTriangles", ShapeBatch_triangles, METH_VARARGS, 0 },
	{ "drawTriangleStrip", ShapeBatch_triangleStrip, METH_VARARGS, 0 },
	{ "drawTriangleFan", ShapeBatch_triangleFan, METH_VARARGS, 0 },
	{ "drawPoints", ShapeBatch_points, METH_VARARGS, 0 },
	{ "drawCircle", ShapeBatch_circle, METH_VARARGS, 0 },
	{ "drawRectangle", ShapeBatch_rectangle, METH_VARARGS, 0 },
	{ "followCamera", (PyCFunction)ShapeBatch_followCamera, METH_VARARGS, 0 },
	{ "ignoreCamera", (PyCFunction)ShapeBatch_ignoreCamera, METH_NOARGS, 0 },
	{ 0 }
};

static PyMemberDef ShapeBatch_members[] = {
	{ "program", T_UINT, offsetof(ShapeBatch, program), READONLY, 0 },
	{ "pointSize", T_FLOAT, offsetof(ShapeBatch, pointSize), 0, 0 },
	{ 0 }
};

static PyGetSetDef ShapeBatch_getset[] = {
	{ "blendMode", (getter)ShapeBatch_getBlendMode, (setter)ShapeBatch_setBlendMode, 0, 0 },
	{ "color255", (getter)ShapeBatch_getColor255, (setter)ShapeBatch_setColor255, 0, 0 },
	{ "color", (getter)ShapeBatch_getColor, (setter)ShapeBatch_setColor, 0, 0 },
	{ "rgb255", (getter)ShapeBatch_get3Color255, (setter)ShapeBatch_set3Color255, 0, 0 },
	{ "alpha", (getter)ShapeBatch_getAlpha, (setter)ShapeBatch_setAlpha, 0, 0 },
	{ 0 }
};

PyTypeObject ShapeBatch_type = {
	PyObject_HEAD_INIT(NULL, 0)
	"glrenderer.ShapeBatch",        /*tp_name*/
	sizeof(ShapeBatch),  /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)ShapeBatch_dealloc, /*tp_dealloc*/
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
	ShapeBatch_methods,
	ShapeBatch_members,
	ShapeBatch_getset,
	0,
	0,
	0,
	0,
	0,
	(initproc)ShapeBatch_init,
	0,
	ShapeBatch_new
};
