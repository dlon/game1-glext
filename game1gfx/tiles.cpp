// TODO: implement TileMapLayer subclass and implement TileMapLayer.drawUsingVBO. from TileMapLayerBase


#include <pyUtil.h>
#include <structmember.h>
#include "GLProgram.h"


const char *tileVShader = R"(#version 440

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


static PyObject *TileMapBaseType = nullptr;


typedef struct {
	PyObject_HEAD
	GLProgram *program;
	GLuint indexVbo;
	GLuint vertexVbo;
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

static int TileMap_init(TileMapObject *self, PyObject *args, PyObject *kwds)
{
	// TODO: setup vboTile
	// TODO: setup index vbo
	return Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);

	/* TODO:
	vboOffset = self._updateArrays()
    # self.vboTileData.resize((vboOffset, 4 * 8), refcheck=False)
    self._updateVBO()
    self._initShaders()
    self._createVAO()
	*/
}

static PyType_Slot TileMap_slots[] = {
	{ Py_tp_base, nullptr }, /* set in tiles_init */
	{ Py_tp_init, TileMap_init },
	{ Py_tp_methods, TileMap_methods },
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

	// setup TileMap type

	PyObject *baseTiles = PyImport_ImportModule("gfx.gl.tiles");
	if (!baseTiles)
		return NULL;
	TileMapBaseType =
		PyObject_GetAttrString(baseTiles, "TileMapBase");
	Py_DECREF(baseTiles);
	if (!TileMapBaseType)
		return NULL;

	TileMap_slots[0].pfunc = TileMapBaseType;
	PyObject *TileMapType =
		PyType_FromSpec(&TileMap_spec);

	if (PyModule_AddObject(
		mod,
		TileMap_spec.name,
		TileMapType) == -1) {
		Py_DECREF(mod);
		return NULL;
	}

	return mod;
}
