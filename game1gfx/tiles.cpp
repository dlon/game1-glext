// TODO: implement TileMapLayer subclass and implement TileMapLayer.drawUsingVBO. from TileMapLayerBase
// TODO: proper cleanup on failure

#include <pyUtil.h>
#include <structmember.h>
#include "GLProgram.h"
#include <vector>
#include "array.h"

#define STRINGIFY(x) #x
#define LOCATION_STR(x) STRINGIFY(x)

// Uniform locations
#define VP_MATRIX_UNIFORM 0
#define TEXTURE0_UNIFORM 1
#define M_MATRIX_UNIFORM 2
#define TEXTURE_SIZE_UNIFORM 3

// Attribute locations
#define POSITION_ATTRIBUTE 0
#define TILE_POSITION_ATTRIBUTE 1
//#define VERTEX_COLOR_ATTRIBUTE 2
#define TILE_SIZE_ATTRIBUTE 3
#define TILE_OFFSET_ATTRIBUTE 4

static const char *tileVShader = "#version 440\n"

"layout(location = " LOCATION_STR(VP_MATRIX_UNIFORM) ") uniform mat3 vpMatrix;\n"
"layout(location = " LOCATION_STR(M_MATRIX_UNIFORM) ") uniform mat3 mMatrix;\n"

"layout(location = " LOCATION_STR(POSITION_ATTRIBUTE) ") in vec2 vPosition;\n"
"layout(location = " LOCATION_STR(TILE_POSITION_ATTRIBUTE) ") in vec2 vTilePos;\n"
//"layout(location = " LOCATION_STR(VERTEX_COLOR_ATTRIBUTE) ") in vec4 vVertColor;\n"
"layout(location = " LOCATION_STR(TILE_SIZE_ATTRIBUTE) ") in vec2 vTileSize;\n"
"layout(location = " LOCATION_STR(TILE_OFFSET_ATTRIBUTE) ") in vec2 vTOffset;\n"

//"out vec4 vertColor;\n"
"out vec2 tilePos;\n"
"out vec2 tileSize;\n"
"out vec2 tOffset;\n"

"out vec2 pos;\n"

"void main(void) {"
"	gl_Position = vec4(vpMatrix * mMatrix * vec3(vPosition, 1.0), 1.0);\n"
//"	vertColor = vVertColor;\n"
"	pos = vPosition;\n"
"	tilePos = vTilePos;\n"
"	tileSize = vTileSize;\n"
"	tOffset = vTOffset;\n"
"}";

static const char *tileFShader = "#version 440\n"

"precision highp float;\n"

//"in vec4 vertColor;\n"
"in vec2 tilePos;\n"
"in vec2 tileSize;\n"
"in vec2 tOffset;\n"

"in vec2 pos;\n"

"layout(location = " LOCATION_STR(TEXTURE0_UNIFORM) ") uniform sampler2D u_texture0;\n"
"layout(location = " LOCATION_STR(TEXTURE_SIZE_UNIFORM) ") uniform vec2 uTexSize;\n"

"layout(location = 0) out vec4 colorResult;\n"

"void main(void) {"
"	colorResult = texture2D("
"		u_texture0,"
"		(tilePos + mod(pos - tOffset, tileSize)) / uTexSize"
"	);\n"
"}";


static PyTypeObject *TileMapBaseType = nullptr;


typedef GLfloat attributeType;
typedef GLushort indexType;


typedef struct {
	PyObject_VAR_HEAD
	GLProgram *program;

	GLuint indexVbo;

	GLuint vertexVbo;
	std::vector<attributeType> *vertexAttribData;

	GLuint vao;

	PyObject *vertexDataObj;
	PyObject *vertexDataView;

	GLfloat camMatrix[3][3];
	float ratio;
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
	float parallax;
	float x, y;

	static char *keywords[] = {
		"parallax",
		"x",
		"y",
		nullptr
	};

	if (!PyArg_ParseTupleAndKeywords(args, kw, "fff", keywords,
		&parallax, &x, &y))
		return NULL;

	self->program->use();

	float xp = x * parallax;
	float yp = y * parallax;

	float ratio = self->ratio;
	self->camMatrix[2][0] = -xp + fmod(xp, ratio);
	self->camMatrix[2][1] = -yp + fmod(yp, ratio);

	glUniformMatrix3fv(
		M_MATRIX_UNIFORM,
		1,
		GL_FALSE,
		(GLfloat*)self->camMatrix
	);

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
	
	self->vertexAttribData = new std::vector<attributeType>;

	return (PyObject*)self;
}

static void TileMap_dealloc(TileMapObject *self) {
	Py_XDECREF(self->vertexDataView);
	Py_XDECREF(self->vertexDataObj);
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
	self->program = program;

	// obtain configured surface size
	PyObject *gfxMod = PyImport_ImportModule("gfx");
	if (!gfxMod)
		return -1;
	PyObject *configobj = PyObject_GetAttrString(gfxMod, "configuration");
	Py_DECREF(gfxMod);
	if (!configobj)
		return -1;
	PyObject *surfSizeobj = PyObject_GetAttrString(configobj, "surfaceSize");
	if (!surfSizeobj) {
		Py_DECREF(configobj);
		return -1;
	}
	PyObject *surfToScreenobj = PyObject_GetAttrString(configobj, "surfaceToScreen");
	Py_DECREF(configobj);
	if (!surfToScreenobj) {
		return -1;
	}
	self->ratio = PyFloat_AsDouble(surfToScreenobj);
	Py_DECREF(surfToScreenobj);

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
		VP_MATRIX_UNIFORM,
		1,
		GL_FALSE,
		(GLfloat*)matrix
	);
	
	self->camMatrix[0][0] = 1;
	self->camMatrix[0][1] = 0;
	self->camMatrix[0][2] = 0;
	self->camMatrix[1][0] = 0;
	self->camMatrix[1][1] = 1;
	self->camMatrix[1][2] = 0;
	self->camMatrix[2][0] = 0;
	self->camMatrix[2][1] = 0;
	self->camMatrix[2][2] = 1;
	/*{ 1, 0, 0 },
	{ 0, 1, 0 },
	{ 0, 0, 1 }*/

	glUniformMatrix3fv(
		M_MATRIX_UNIFORM,
		1,
		GL_FALSE,
		(GLfloat*)self->camMatrix
	);

	glUniform1i(TEXTURE0_UNIFORM, 0);

	return 0;
}

static int TileMap_setupVAO(TileMapObject *self)
{
	glGenVertexArrays(1, &self->vao);
	glBindVertexArray(self->vao);

	self->program->use();

	glEnableVertexAttribArray(POSITION_ATTRIBUTE);
	glEnableVertexAttribArray(TILE_POSITION_ATTRIBUTE);
	glEnableVertexAttribArray(TILE_SIZE_ATTRIBUTE);
	glEnableVertexAttribArray(TILE_OFFSET_ATTRIBUTE);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->indexVbo);
	glBindBuffer(GL_ARRAY_BUFFER, self->vertexVbo);

	glVertexAttribPointer(
		TILE_OFFSET_ATTRIBUTE,
		2,
		GL_FLOAT,
		GL_FALSE,
		8 * sizeof(attributeType),
		(const GLvoid*)(0 * sizeof(attributeType))
	);
	glVertexAttribPointer(
		TILE_POSITION_ATTRIBUTE,
		2,
		GL_FLOAT,
		GL_FALSE,
		8 * sizeof(attributeType),
		(const GLvoid*)(2 * sizeof(attributeType))
	);
	glVertexAttribPointer(
		TILE_SIZE_ATTRIBUTE,
		2,
		GL_FLOAT,
		GL_FALSE,
		8 * sizeof(attributeType),
		(const GLvoid*)(4 * sizeof(attributeType))
	);
	glVertexAttribPointer(
		POSITION_ATTRIBUTE,
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
	std::vector<indexType> indices;
	indices.resize(6 * maxTileBatch);
	for (int index = 0; index < maxTileBatch; index++) {
		indices[6 * index + 0] = 4 * index + 0;
		indices[6 * index + 1] = 4 * index + 2;
		indices[6 * index + 2] = 4 * index + 1;
		indices[6 * index + 3] = 4 * index + 2;
		indices[6 * index + 4] = 4 * index + 1;
		indices[6 * index + 5] = 4 * index + 3;
	}

	glGenBuffers(1, &self->indexVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->indexVbo);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		sizeof(indexType) * indices.size(),
		indices.data(),
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
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
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
		PyType_FromSpecWithBases(&TileMap_spec, nullptr);

	if (PyModule_AddObject(
		mod,
		TileMap_spec.name,
		TileMapType) == -1) {
		Py_DECREF(mod);
		return NULL;
	}

	return mod;
}
