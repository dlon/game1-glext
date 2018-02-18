#include <Python.h>
#include <structmember.h>
#include "glutil.hpp"

typedef struct {
	PyObject_HEAD
	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint program;
	GLuint vbo;
	GLuint vao;
	unsigned int maxVertices;
	PyObject *color;
	int type;
	int vertCount;
	GLfloat *vertexData;
} ShapeBatch;

/*
shapeVertexPositionAttribute = 0
shapeVertexColorAttribute = 1

shapeVMatrixUniform = 0
shapeMMatrixUniform = 1
shapePointSizeUniform = 2
*/

static const char vertexShaderSource[] =
"#version 440"

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
"#version 440"

"in vec4 vertColor;"

"void main(void) {"
"	gl_FragColor = vertColor;"
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
	printShaderInfoLog(self->vertexShader);
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
	printProgramInfoLog(self->program);

	glEnableVertexAttribArray(0); // vertex position attrib
	glEnableVertexAttribArray(1); // vertex color attrib

	GLfloat projectionMatrix[3][3] = {
		{ 2.0f / 320.0f, 0, 0 },
		{ 0, -2.0f / 240.0f, 0 },
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

	GLfloat mMatrix[3][3] = {
		{ 1, 0, 0 },
		{ 0, 1, 0 },
		{ 0, 0, 1 }
	};

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
		(GLfloat*)mMatrix
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

	return 0;
}

static PyObject *ShapeBatch_begin(ShapeBatch *self, PyObject *args)
{
	glUseProgram(self->program);
	Py_RETURN_NONE;
}

static void end(ShapeBatch *self)
{
	if (!self->type)
		return;

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

static PyObject *ShapeBatch_end(ShapeBatch *self, PyObject *args)
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

	float r = PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(self->color, 0));
	float g = PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(self->color, 1));
	float b = PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(self->color, 2));
	float a = PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(self->color, 3));

	for (Py_ssize_t i = ppCount; i < pCount; i++)
	{
		x = PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(args, 2 * (i - ppCount) + 0));
		y = PyFloat_AS_DOUBLE(PyTuple_GET_ITEM(args, 2 * (i - ppCount) + 1));
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
	Py_RETURN_NONE;
}

static PyMethodDef ShapeBatch_methods[] = {
	{ "begin", (PyCFunction)ShapeBatch_begin, METH_NOARGS, 0 },
	{ "end", (PyCFunction)ShapeBatch_end, METH_NOARGS, 0 },
	{ "drawLineStrip", ShapeBatch_lineStrip, METH_VARARGS, 0 },
	{ 0 }
};

static PyMemberDef ShapeBatch_members[] = {
	{ "color", T_OBJECT_EX, offsetof(ShapeBatch, color), 0, 0 },
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
	0,
	0,
	0,
	0,
	0,
	0,
	(initproc)ShapeBatch_init,
	0,
	ShapeBatch_new
};
