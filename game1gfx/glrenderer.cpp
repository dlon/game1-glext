#include <Python.h>
#include <structmember.h>
#include "glutil.hpp"
#include <Windows.h>
#include <gl/GL.h>
#include "Texture.h"
#include "TextureRegion.h"
#include "batch.hpp"
#include "shapes.h"
#include "entity.h"
#include "glrenderer.h"


extern "C" float surfaceWidth;
extern "C" float surfaceHeight;
float surfaceWidth;
float surfaceHeight;

static PyObject* Batch_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	glrenderer_Batch *self;
	self = (glrenderer_Batch*)type->tp_alloc(type, 0);
	if (self != NULL) {
		/*self->first = PyUnicode_FromString("");
		if (self->first == NULL) {
			Py_DECREF(self);
			return NULL;
		}*/
		// TODO: catch exception and return NULL
		self->_object = new Batch(1000, surfaceWidth, surfaceHeight);
	}
	return (PyObject*)self;
}

static int Batch_init(glrenderer_Batch *self, PyObject *args, PyObject *kwds) {
	size_t batchSize;
	if (!PyArg_ParseTuple(args, "I", &batchSize))
		return -1;
	return 0;
}

static void Batch_dealloc(glrenderer_Batch* self) {
	//Py_XDECREF(self->first);
	delete self->_object;
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *
Batch_begin(glrenderer_Batch *self)
{
	self->_object->begin();
	Py_RETURN_NONE;
}

static PyObject *
Batch_end(glrenderer_Batch *self)
{
	self->_object->end();
	Py_RETURN_NONE;
}

static PyObject *
Batch_flush(glrenderer_Batch *self)
{
	self->_object->flush();
	Py_RETURN_NONE;
}

static PyObject *
Batch_draw(glrenderer_Batch *self, PyObject *args, PyObject *kwds)
{
	float x = 0;
	float y = 0;
	glrenderer_TextureRegion* region = NULL;

	static char *kwlist[] = { "textureRegion", "x", "y", 0 };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!ff", kwlist,
		&glrenderer_TextureRegionType, &region,
		&x, &y))
		return 	NULL;
	
	// TODO: error checking
	// TODO: memory handling?
	self->_object->draw(*(region->_object), x, y);

	Py_RETURN_NONE;
}

static PyObject *
Batch_getMaxQuads(glrenderer_Batch *self, void *closure)
{
	return PyLong_FromLong(self->_object->getBatchSize());
}

static PyObject *
Batch_getProgram(glrenderer_Batch *self, void *closure)
{
	return PyLong_FromSize_t(self->_object->getProgram());
}

static PyObject *
Batch_getBlendMode(glrenderer_Batch *self, void *closure)
{
	GLenum mode[2];
	self->_object->getBlendMode(mode);
	return Py_BuildValue(
		"II",
		mode[0],
		mode[1]
	);
}

static int
Batch_setBlendMode(glrenderer_Batch *self, PyObject *args, void *closure)
{
	GLenum mode[2];
	if (!PyArg_ParseTuple(args, "II",
		&mode[0], &mode[1]))
		return -1; // TODO: set an exception
	self->_object->setBlendMode(mode[0], mode[1]);
	return 0;
}

static PyObject *
Batch_getObjectIndex(glrenderer_Batch *self, void *closure)
{
	return PyLong_FromLong(self->_object->getObjectIndex());
}

static PyObject *
Batch_ignoreCamera(glrenderer_Batch *self)
{
	self->_object->ignoreCamera();
	Py_RETURN_NONE;
}

static PyObject *
Batch_followCamera(glrenderer_Batch *self, PyObject *args, PyObject *kwds)
{
	float parallax;
	float x, y;
	if (!PyArg_ParseTuple(args, "fff",
		&parallax, &x, &y))
		return NULL;

	self->_object->followCamera(parallax, x, y);

	Py_RETURN_NONE;
}

static PyMethodDef Batch_methods[] = {
	{ "begin", (PyCFunction)Batch_begin, METH_NOARGS, NULL },
	{ "flush", (PyCFunction)Batch_flush, METH_NOARGS, NULL },
	{ "end", (PyCFunction)Batch_end, METH_NOARGS, NULL },
	{ "draw", (PyCFunction)Batch_draw, METH_VARARGS | METH_KEYWORDS, NULL },
	{ "ignoreCamera", (PyCFunction)Batch_ignoreCamera, METH_NOARGS, NULL },
	{ "followCamera", (PyCFunction)Batch_followCamera, METH_VARARGS, NULL },
	{ NULL }
};

static PyMemberDef Batch_members[] = {
	{ NULL }
};

static PyGetSetDef Batch_getset[] = {
	{ "maxQuads",  (getter)Batch_getMaxQuads, 0, 0, 0 },
	{ "program",  (getter)Batch_getProgram, 0, 0, 0 },
	{ "blendMode",  (getter)Batch_getBlendMode, (setter)Batch_setBlendMode, 0, 0 },
	{ "objectIndex",  (getter)Batch_getObjectIndex, 0, 0, 0 },
	{ NULL }
};

static PyTypeObject glrenderer_BatchType = {
	PyObject_HEAD_INIT(NULL, 0)
	"glrenderer.Batch",        /*tp_name*/
	sizeof(glrenderer_Batch),  /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)Batch_dealloc, /*tp_dealloc*/
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
	Batch_methods,
	Batch_members,
	Batch_getset,
	0,
	0,
	0,
	0,
	0,
	(initproc)Batch_init,
	0,
	Batch_new
};

static PyObject* glrenderer_init(PyObject *self, PyObject *args) {
	if (!PyArg_ParseTuple(args, "ff",
		&surfaceWidth, &surfaceHeight))
		return NULL;

	loadGLFunctions();

	Py_RETURN_NONE;
}

// when a function fails, it should set an exception condition and return an error value (usually a NULL pointer)
// Exceptions are stored in a static global variable inside the interpreter; if this variable is NULL no exception has occurred.
// A second global variable stores the “associated value” of the exception (the second argument to raise).
// A third variable contains the stack traceback in case the error originated in Python code.
// These three variables are the C equivalents of the Python variables sys.exc_type, sys.exc_value
// and sys.exc_traceback (see the section on module sys in the Python Library Reference).

// set exceptions: PyErr_SetString(), PyErr_SetFromErrno(), PyErr_SetObject(). No need for Py_INCREF()
// exception test: PyErr_Occurred(). Normally not needed due to the NULL return value.
// there are built-in exceptions, such as PyExc_ZeroDivisionError

// if f calls g and g fails, return NULL or -1. don't use PyErr_*
// if it wants to destroy the exception, it should call PyErr_Clear().
// if malloc()/realloc() fails, call PyErr_NoMemory (the direct caller).

// be careful to clean up garbage (by making Py_XDECREF() or Py_DECREF() calls for objects you have already created)
// when you return an error indicator!

// method table and initialization function


#include "ParticleSystemCollection.h"
#include "ParticleSystemRenderer.h"
#include "gfx.h"
#include "shapes.h"
#include "particles.h"

static ParticleSystemRenderer *particleRenderer = nullptr;
static ParticleSystemCollection *particleSystemCollection = nullptr;

PyObject * glrenderer_drawParticleSystem(PyObject *self, PyObject *args, PyObject *kw)
{
	char *keywords[] = {
		"renderer",
		"particleSystem",
		NULL
	};
	PyObject *renderer;
	ParticleSystemObject *sysObj;

	if (!PyArg_ParseTupleAndKeywords(
		args, kw, "OO!", keywords,
		&renderer,
		&ParticleSystemType, &sysObj))
		return NULL;

	glrenderer_ShapeBatch *shapes = getShapeBatch(renderer);
	Batch *batch = getRendererBatch(renderer);

	particleRenderer->render(shapes, batch, sysObj->system);

	Py_RETURN_NONE;
}

PyObject * glrenderer_drawParticles(PyObject *self, PyObject *renderer)
{
	glrenderer_ShapeBatch *shapes = getShapeBatch(renderer);
	Batch *batch = getRendererBatch(renderer);

	particleRenderer->render(shapes, batch, *particleSystemCollection);

	Py_RETURN_NONE;
}


static PyMethodDef methods[] = {
	{ "init", glrenderer_init, METH_VARARGS, NULL },
	{ "drawParticles", glrenderer_drawParticles, METH_O, NULL },
	{ "drawParticleSystem", (PyCFunction)glrenderer_drawParticleSystem, METH_VARARGS | METH_KEYWORDS, NULL },
	NULL
};

static void glrenderer_free(void *ptr) {
	if (particleRenderer) {
		delete particleRenderer;
		particleRenderer = nullptr;
	}
	entity_cleanup();
	gfx_cleanup();
}

static struct PyModuleDef glextModule = {
	PyModuleDef_HEAD_INIT,
	"glrenderer",   /* name of module */
	NULL, /* module documentation, may be NULL */
	-1,       /* size of per-interpreter state of the module,
			  or -1 if the module keeps state in global variables. */
	methods,
	NULL, // m_slots
	NULL, // m_traverse (during gc traversal)
	NULL, // m_clear (gc clearing)
	glrenderer_free
};

PyObject * glrenderer_GraphicsError = nullptr;

PyMODINIT_FUNC
PyInit_glrenderer(void)
{
	PyObject *m;

	if (PyType_Ready(&glrenderer_BatchType) < 0)
		return NULL;
	if (PyType_Ready(&glrenderer_TextureType) < 0)
		return NULL;
	if (PyType_Ready(&glrenderer_TextureRegionType) < 0)
		return NULL;
	if (PyType_Ready(&ShapeBatch_type) < 0)
		return NULL;

	glrenderer_GraphicsError = PyErr_NewException(
		"glrenderer.GraphicsError",
		PyExc_RuntimeError, NULL);

	m = PyModule_Create(&glextModule);
	if (m == NULL)
		return NULL;

	if (!gfx_init())
		return NULL;

	if (entity_init(m) == -1) {
		Py_DECREF(m);
		return NULL;
	}

	//loadGLFunctions(); // TODO: fix using fake WGL context

	Py_INCREF((PyObject *)&glrenderer_BatchType);
	Py_INCREF((PyObject *)&glrenderer_TextureType);
	Py_INCREF((PyObject *)&glrenderer_TextureRegionType);
	Py_INCREF((PyObject *)&ShapeBatch_type);

	PyModule_AddObject(m, "Batch", (PyObject *)&glrenderer_BatchType);
	PyModule_AddObject(m, "Texture", (PyObject *)&glrenderer_TextureType);
	PyModule_AddObject(m, "TextureRegion", (PyObject *)&glrenderer_TextureRegionType);
	PyModule_AddObject(m, "ShapeBatch", (PyObject *)&ShapeBatch_type);

	PyObject *coreMod = PyImport_ImportModule("core");
	PyObject *particlesMod = PyObject_GetAttrString(coreMod, "particles");
	Py_DECREF(coreMod);
	PyObject *psysCollectionObj = PyObject_GetAttrString(particlesMod, "_collection");
	Py_DECREF(particlesMod);
	if (!psysCollectionObj) {
		return NULL;
	}
	particleSystemCollection =
		(ParticleSystemCollection*)PyCapsule_GetPointer(psysCollectionObj, NULL);
	Py_DECREF(psysCollectionObj);
	if (!particleSystemCollection) {
		return NULL;
	}

	particleRenderer = new ParticleSystemRenderer;

	return m;
}
