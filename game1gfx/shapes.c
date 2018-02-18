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

	self->program = glCreateProgram();

	glAttachShader(self->program, self->vertexShader);
	glAttachShader(self->program, self->fragmentShader);
	glLinkProgram(self->program);

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
	ShapeBatch *self = type->tp_alloc(type, sizeof(ShapeBatch));
	if (self) {
		self->color = PyTuple_Pack(4, 1, 1, 1, 1);
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
	Py_TYPE(self)->tp_dealloc(self);
}

static int ShapeBatch_init(ShapeBatch *self, PyObject *args, PyObject *kwds)
{
	static char *kwlist[] = { "maxVertices" };

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

static PyObject *ShapeBatch_end(ShapeBatch *self, PyObject *args)
{
	if (!self->type)
		Py_RETURN_NONE;

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

	Py_RETURN_NONE;
}

static PyMethodDef ShapeBatch_methods[] = {
	{ "begin", (PyCFunction)ShapeBatch_begin, METH_NOARGS, 0 },
	{ "end", (PyCFunction)ShapeBatch_end, METH_NOARGS, 0 },
	{ 0 }
};

static PyMemberDef ShapeBatch_members[] = {
	{ "color", T_OBJECT_EX, offsetof(ShapeBatch, color), 0, 0 },
	{ NULL }
};

static PyTypeObject ShapeBatch_type = {
	PyVarObject_HEAD_INIT(NULL, 0)
	sizeof(ShapeBatch),             /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)ShapeBatch_dealloc, /* tp_dealloc */
	0,                         /* tp_print */
	0,                         /* tp_getattr */
	0,                         /* tp_setattr */
	0,                         /* tp_reserved */
	0,                         /* tp_repr */
	0,                         /* tp_as_number */
	0,                         /* tp_as_sequence */
	0,                         /* tp_as_mapping */
	0,                         /* tp_hash  */
	0,                         /* tp_call */
	0,                         /* tp_str */
	0,                         /* tp_getattro */
	0,                         /* tp_setattro */
	0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT |
	Py_TPFLAGS_BASETYPE,   /* tp_flags */
	0,           /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	ShapeBatch_methods,             /* tp_methods */
	ShapeBatch_members,             /* tp_members */
	0,                         /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)ShapeBatch_init,      /* tp_init */
	0,                         /* tp_alloc */
	ShapeBatch_new,                 /* tp_new */
};
