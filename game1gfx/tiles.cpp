// TODO: implement TileMapLayer subclass and implement TileMapLayer.drawUsingVBO. from TileMapLayerBase


#include <pyUtil.h>
#include <structmember.h>
#include "GLProgram.h"
#include <vector>


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
} TileMapObject;

static PyObject *TileMap_delete(TileMapObject *self, PyObject *noarg)
{
	// TODO
	Py_RETURN_NONE;
}

static PyObject *TileMap_refresh(TileMapObject *self, PyObject *noarg)
{
	// TODO
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

static PyObject* TileMap_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	TileMapObject *self = (TileMapObject*)type->tp_alloc(type, 0);
	if (self == NULL)
		return NULL;
	
	self->indices = new std::vector<indexType>;
	self->vertexAttribData = new std::vector<attributeType>;

	return (PyObject*)self;
}

static void TileMap_dealloc(TileMapObject *self) {
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

	// FIXME: self->vertexAttribData must be accessible as a 2D array in Python
	// the vertex data must be set up before the base; the rest later
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
	// TODO: create VAO
	// TODO: set up shaders

	return 0;
}

static PyType_Slot TileMap_slots[] = {
	{ Py_tp_base, nullptr }, /* set in tiles_init */
	{ Py_tp_init, TileMap_init },
	{ Py_tp_methods, TileMap_methods },
	{ Py_tp_new, TileMap_new },
	{ Py_tp_dealloc, TileMap_dealloc },
	{ 0 },
};
static PyType_Spec TileMap_spec = {
	"TileMap",
	sizeof(TileMapObject),
	0,
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
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
