// TODO: implement TileMapLayer subclass and implement TileMapLayer.drawUsingVBO. from TileMapLayerBase
// TODO: proper cleanup on failure

#include <pyUtil.h>
#include <structmember.h>
#include "GLProgram.h"
#include <vector>
#include "array.h"


static const char *tileVShader = R"(#version 440

layout(location = 0) uniform mat3 vpMatrix;
layout(location = 2) uniform mat3 mMatrix;

layout(location = 0) in vec2 vPosition;
layout(location = 1) in vec2 vTilePos;
//layout(location = 2) in vec4 vVertColor;
layout(location = 3) in vec2 vTileSize;
layout(location = 4) in vec2 vTOffset;

//out vec4 vertColor;
out vec2 tilePos;
out vec2 tileSize;
out vec2 tOffset;

out vec2 pos;

void main(void) {
	gl_Position = vec4(vpMatrix * mMatrix * vec3(vPosition, 1.0), 1.0);
	//vertColor = vVertColor;
	pos = vPosition;
	tilePos = vTilePos;
	tileSize = vTileSize;
	tOffset = vTOffset;
}
)";

static const char *tileFShader = R"(#version 440

precision highp float;

//in vec4 vertColor;
in vec2 tilePos;
in vec2 tileSize;
in vec2 tOffset;

in vec2 pos;

layout(location = 1) uniform sampler2D u_texture0;
layout(location = 3) uniform vec2 uTexSize;

layout(location = 0) out vec4 colorResult;

void main(void) {
	colorResult = texture2D(
		u_texture0,
		(tilePos + mod(pos - tOffset, tileSize)) / uTexSize
	);
}
)";


static PyTypeObject *TileMapBaseType = nullptr;


typedef GLfloat attributeType;
typedef GLushort indexType;


typedef struct {
	PyObject_HEAD
	GLProgram *program;

	GLuint indexVbo;
	std::vector<indexType> *indices;

	GLuint vertexVbo;
	std::vector<attributeType> *vertexAttribData;

	unsigned int vao;

	PyObject *vertexDataObj;
	PyObject *vertexDataView;
} TileMapObject;

static void TileMap_updateVerticesVBO(TileMapObject *self);


static PyObject *TileMap_delete(TileMapObject *self, PyObject *noarg)
{
	// TODO: why not in dealloc?
	glDeleteBuffers(1, &self->vertexVbo);
	glDeleteBuffers(1, &self->indexVbo);
	glDeleteVertexArrays(1, &self->vao);
	Py_RETURN_NONE;
}

static PyObject *TileMap_refresh(TileMapObject *self, PyObject *noarg)
{
	PyTypeObject *base = Py_TYPE(self)->tp_base;
	PyObject *ret = PyObject_CallMethod((PyObject*)base, "_updateArrays", "O", self);
	if (!ret)
		return NULL;
	Py_DECREF(ret);

	TileMap_updateVerticesVBO(self);

	Py_RETURN_NONE;
}

static PyObject *TileMap_followCamera(TileMapObject *self, PyObject *args, PyObject *kw)
{
	// TODO
	Py_RETURN_NONE;
}

PyMethodDef TileMap_methods[] = {
	{ "delete", (PyCFunction)TileMap_delete, METH_NOARGS, NULL },
	{ "refresh", (PyCFunction)TileMap_refresh, METH_NOARGS, NULL },
	{ "followCamera", (PyCFunction)TileMap_followCamera, METH_VARARGS | METH_KEYWORDS, NULL },
	NULL
};

PyMemberDef TileMap_members[] = {
	{ "vboTileData", T_OBJECT, offsetof(TileMapObject, vertexDataView), READONLY, 0 },
	{ "vao", T_UINT, offsetof(TileMapObject, vao), READONLY, 0 },
	{ NULL }
};

static PyObject *TileMap_getProgram(TileMapObject *self, void *closure)
{
	return PyLong_FromUnsignedLong(self->program->getGLProgram());
}

PyGetSetDef TileMap_getset[] = {
	{ "program", (getter)TileMap_getProgram, (setter)nullptr, 0, 0 },
	{ NULL }
};

static PyObject* TileMap_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	TileMapObject *self = (TileMapObject*)type->tp_alloc(type, 0);
	if (self == NULL)
		return NULL;
	
	self->indices = new std::vector<indexType>;
	self->vertexAttribData = new std::vector<attributeType>;

	return (PyObject*)self;
}

static void TileMap_dealloc(TileMapObject *self) {
	Py_XDECREF(self->vertexDataView);
	Py_XDECREF(self->vertexDataObj);
	delete self->indices;
	delete self->vertexAttribData;
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static void TileMap_updateVerticesVBO(TileMapObject *self)
{
	glBindBuffer(
		GL_ARRAY_BUFFER,
		self->vertexVbo
	);
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(attributeType) * self->vertexAttribData->size(),
		self->vertexAttribData->data(),
		GL_STATIC_DRAW
	);
}

static int TileMap_setupShaders(TileMapObject *self)
{
	GLProgram *program = new GLProgram(
		tileVShader,
		tileFShader,
		nullptr
	);
	if (!program->OK()) {
		PyErr_SetString(
			PyExc_RuntimeError,
			program->getErrorMessage()
		);
		delete program;
		return -1;
	}

	// TODO: use compile-time values
	const int texUniform = program->getUniformLocation("u_texture0");
	const int texSizeUniform = program->getUniformLocation("uTexSize");
	const int modelUniform = program->getUniformLocation("mMatrix");
	const int viewProjectionUniform = program->getUniformLocation("vpMatrix");

	// obtain configured surface size
	PyObject *gfxMod = PyImport_ImportModule("gfx");
	if (!gfxMod)
		return -1;
	PyObject *configobj = PyObject_GetAttrString(gfxMod, "configuration");
	Py_DECREF(gfxMod);
	if (!configobj)
		return -1;
	PyObject *surfSizeobj = PyObject_GetAttrString(configobj, "surfaceSize");
	Py_DECREF(configobj);
	if (!surfSizeobj)
		return -1;

	PyObject *widthobj = PySequence_GetItem(surfSizeobj, 0);
	PyObject *heightobj = PySequence_GetItem(surfSizeobj, 1);
	Py_DECREF(surfSizeobj);
	if (!widthobj || !heightobj) {
		Py_XDECREF(widthobj);
		Py_XDECREF(heightobj);
		return -1;
	}
	float width = PyFloat_AsDouble(widthobj);
	float height = PyFloat_AsDouble(heightobj);
	Py_XDECREF(widthobj);
	Py_XDECREF(heightobj);

	// set up matrices

	GLfloat projectionMatrix[3][3] = {
		{ 2.0f / width, 0, 0 },
		{ 0, -2.0f / height, 0 },
		{ 0, 0, 1 }
	};
	GLfloat viewMatrix[3][3] = {
		{ 1, 0, 0 },
		{ 0, 1, 0 },
		{ -160, -120, 1 }
	};

	GLfloat matrix[3][3] = { 0 };
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 3; k++) {
				matrix[i][j] += viewMatrix[i][j] * projectionMatrix[k][j];
			}
		}
	}

	program->use();

	glUniformMatrix3fv(
		viewProjectionUniform,
		1,
		GL_FALSE,
		(GLfloat*)matrix
	);
	
	GLfloat camMatrix[3][3] = {
			{ 1, 0, 0 },
			{ 0, 1, 0 },
			{ 0, 0, 1 }
	};
	glUniformMatrix3fv(
		modelUniform,
		1,
		GL_FALSE,
		(GLfloat*)camMatrix
	);

	glUniform1i(texUniform, 0);

	return 0;
}

static int TileMap_setupVAO(TileMapObject *self)
{
	glGenVertexArrays(1, &self->vao);
	glBindVertexArray(self->vao);

	self->program->use();

	// TODO: at compile-time
	const int positionAttrib = self->program->getAttribLocation("vPosition");
	const int tilePosAttrib = self->program->getAttribLocation("vTilePos");
	const int tileSizeAttrib = self->program->getAttribLocation("vTileSize");
	const int tOffsetAttrib = self->program->getAttribLocation("vTOffset");

	glEnableVertexAttribArray(tOffsetAttrib);
	glEnableVertexAttribArray(tilePosAttrib);
	glEnableVertexAttribArray(tileSizeAttrib);
	glEnableVertexAttribArray(positionAttrib);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->indexVbo);
	glBindBuffer(GL_ARRAY_BUFFER, self->vertexVbo);

	glVertexAttribPointer(
		tOffsetAttrib,
		2,
		GL_FLOAT,
		GL_FALSE,
		8 * sizeof(attributeType),
		(const GLvoid*)(0 * sizeof(attributeType))
	);
	glVertexAttribPointer(
		tilePosAttrib,
		2,
		GL_FLOAT,
		GL_FALSE,
		8 * sizeof(attributeType),
		(const GLvoid*)(2 * sizeof(attributeType))
	);
	glVertexAttribPointer(
		tileSizeAttrib,
		2,
		GL_FLOAT,
		GL_FALSE,
		8 * sizeof(attributeType),
		(const GLvoid*)(4 * sizeof(attributeType))
	);
	glVertexAttribPointer(
		positionAttrib,
		2,
		GL_FLOAT,
		GL_FALSE,
		8 * sizeof(attributeType),
		(const GLvoid*)(6 * sizeof(attributeType))
	);

	glBindVertexArray(0);
	return 0;
}

static int TileMap_init(TileMapObject *self, PyObject *args, PyObject *kwds)
{
	char *keywords[] = {
		"tiles",
		nullptr
	};
	PyObject *tiles;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", keywords, &tiles)) {
		return -1;
	}

	// the vertex array must be set up before the base; the vertex VBO after
	PyObject *maxTileBatchObj =
		PyObject_GetAttrString((PyObject*)self, "maxTileBatch");
	if (!maxTileBatchObj)
		return -1;
	int maxTileBatch = PyLong_AsLong(maxTileBatchObj);
	Py_DECREF(maxTileBatchObj);
	if (PyErr_Occurred())
		return -1;
	
	self->vertexAttribData->resize(4 * 8 * maxTileBatch);
	glGenBuffers(1, &self->vertexVbo);

	// create view for vertex data
	Py_ssize_t shape[] = { maxTileBatch, 4 * 8 };
	self->vertexDataObj = glrenderer_CArray_New(
		self->vertexAttribData->data(),
		"f",
		sizeof(attributeType),
		2,
		shape
	);
	if (!self->vertexDataObj)
		return -1;
	self->vertexDataView =
		PyMemoryView_FromObject(self->vertexDataObj);
	if (!self->vertexDataView)
		return -1;

	// TODO: can we forgo this indirect call with a heap type object?
	//       calling tp_base->tp_init causes recursion error
	PyTypeObject *base = Py_TYPE(self)->tp_base;
	if (!PyObject_CallMethod((PyObject*)base, "__init__", "OO", self, tiles))
		return -1;

	// set up index vbo
	glGenBuffers(1, &self->indexVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->indexVbo);

	self->indices->resize(6 * maxTileBatch);
	for (int index = 0; index < maxTileBatch; index++) {
		(*self->indices)[4 * index + 0] = 4 * index + 0;
		(*self->indices)[4 * index + 1] = 4 * index + 2;
		(*self->indices)[4 * index + 2] = 4 * index + 1;
		(*self->indices)[4 * index + 3] = 4 * index + 2;
		(*self->indices)[4 * index + 4] = 4 * index + 1;
		(*self->indices)[4 * index + 5] = 4 * index + 3;
	}
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		sizeof(indexType) * self->indices->size(),
		self->indices->data(),
		GL_STATIC_DRAW
	);

	TileMap_updateVerticesVBO(self);

	if (TileMap_setupShaders(self)) {
		return -1;
	}

	if (TileMap_setupVAO(self)) {
		return -1;
	}

	return 0;
}

static PyType_Slot TileMap_slots[] = {
	{ Py_tp_base, nullptr }, /* set in tiles_init */
	{ Py_tp_init, TileMap_init },
	{ Py_tp_methods, TileMap_methods },
	{ Py_tp_getset, TileMap_getset },
	{ Py_tp_members, TileMap_members },
	{ Py_tp_new, TileMap_new },
	{ Py_tp_dealloc, TileMap_dealloc },
	{ 0 },
};
static PyType_Spec TileMap_spec = {
	"TileMap",
	sizeof(TileMapObject),
	0,
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_TYPE_SUBCLASS,
	TileMap_slots
};

static void tileMod_free(void *ptr) {
	if (TileMapBaseType) {
		Py_DECREF(TileMapBaseType);
		TileMapBaseType = nullptr;
	}
}

static PyModuleDef tileModDef = {
	PyModuleDef_HEAD_INIT,
	"tiles", // m_name
	NULL, // m_doc
	0, // m_size
	0, // m_methods
	NULL, // m_slots
	NULL, // m_traverse (during gc traversal)
	NULL, // m_clear (gc clearing)
	tileMod_free
};

PyMODINIT_FUNC
tiles_init()
{
	PyObject *mod = PyModule_Create(&tileModDef);
	if (!mod)
		return NULL;

	// set up TileMap type
	PyObject *baseTiles = PyImport_ImportModule("gfx.gl.tiles");
	if (!baseTiles)
		return NULL;
	TileMapBaseType = (PyTypeObject*)
		PyObject_GetAttrString(baseTiles, "TileMapBase");
	Py_DECREF(baseTiles);
	if (!TileMapBaseType)
		return NULL;

	TileMap_slots[0].pfunc = TileMapBaseType;
	PyObject *TileMapType =
		PyType_FromSpec(&TileMap_spec);

	if (PyType_Ready((PyTypeObject*)TileMapType) < 0)
		return NULL;

	if (PyModule_AddObject(
		mod,
		TileMap_spec.name,
		TileMapType) == -1) {
		Py_DECREF(mod);
		return NULL;
	}

	return mod;
}
