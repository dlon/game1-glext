#include "entity/DefaultEntityRenderer.h"
#include "entity/particles.h"
#include <Python.h>

PyObject *entity_glMod = nullptr;
PyObject *entity_deepcopy = nullptr;

static int initDefaultRenderer()
{
	entity_glMod = PyImport_ImportModule("gfx.gl");
	if (!entity_glMod)
		return -1;
	
	// import copy.deepcopy
	PyObject *copyMod = PyImport_ImportModule("copy");
	if (!copyMod)
		return NULL;
	entity_deepcopy = PyObject_GetAttrString(copyMod, "deepcopy");
	Py_DECREF(copyMod);
	if (!entity_deepcopy)
		return NULL;

	if (PyType_Ready(&DefaultEntityRendererType) < 0)
		return -1;

	return 0;
}

int entity_init(PyObject *glExtModule)
{
	// TODO: create a submodule "entity" to contain the renderers

	if (initDefaultRenderer() < 0)
		return -1;
	if (PyType_Ready(&ParticleGroupRendererType) < 0)
		return -1;

	PyModule_AddObject(glExtModule, DefaultEntityRendererType.tp_name, (PyObject *)&DefaultEntityRendererType);
	PyModule_AddObject(glExtModule, ParticleGroupRendererType.tp_name, (PyObject *)&ParticleGroupRendererType);

	return 0;
}

int entity_cleanup()
{
	if (entity_deepcopy) {
		Py_DECREF(entity_deepcopy);
		entity_deepcopy = nullptr;
	}
	if (entity_glMod) {
		Py_DECREF(entity_glMod);
		entity_glMod = nullptr;
	}
	return 0;
}

PyObject* getEntityRegion(PyObject *entity)
{
	PyObject *sprites = PyObject_GetAttrString(entity, "_sprites");
	if (PyObject_Not(sprites)) {
		Py_DECREF(sprites);
		Py_RETURN_NONE;
	}

	PyObject *spriteName = PyObject_GetAttrString(entity, "sprite");
	if (!spriteName) {
		PyErr_Clear();
		spriteName = PyUnicode_FromString("default");
	}

	PyObject *region = PyDict_GetItem(sprites, spriteName);
	Py_DECREF(spriteName);
	Py_DECREF(sprites);
	if (!region) {
		// TODO: throw GraphicsError
		PyErr_Format(PyExc_SystemError, "%R has no sprite '%S'.", entity, spriteName);
		return NULL;
	}

	return region;
}

int getEntityPos(PyObject *entity, float *x, float *y)
{
	PyObject *xObj = PyObject_GetAttrString(entity, "x");
	if (!xObj)
		return -1;
	PyObject *yObj = PyObject_GetAttrString(entity, "y");
	if (!yObj) {
		Py_DECREF(xObj);
		return -1;
	}
	*x = PyFloat_AsDouble(xObj);
	*y = PyFloat_AsDouble(yObj);
	Py_DECREF(xObj);
	Py_DECREF(yObj);
	return 0;
}

void getEntityImageOffset(PyObject *entity, float *x, float *y)
{
	PyObject *imgOffsetObj = PyObject_GetAttrString(entity, "imageOffset");
	if (!imgOffsetObj) {
		PyErr_Clear();
		*x = 0.0f;
		*y = 0.0f;
		return;
	}
	PyObject *xObj = PySequence_GetItem(imgOffsetObj, 0);
	PyObject *yObj = PySequence_GetItem(imgOffsetObj, 1);
	*x = PyFloat_AsDouble(xObj);
	*y = PyFloat_AsDouble(yObj);
	Py_DECREF(xObj);
	Py_DECREF(yObj);
}

void getEntityParallax(PyObject *entity, float *x, float *y)
{
	PyObject *parallax = PyObject_GetAttrString(entity, "parallax");
	if (!parallax) {
		PyErr_Clear();
		*x = 1.0f;
		*y = 1.0f;
		return;
	}
	PyObject *xObj = PySequence_GetItem(parallax, 0);
	PyObject *yObj = PySequence_GetItem(parallax, 1);
	*x = PyFloat_AsDouble(xObj);
	*y = PyFloat_AsDouble(yObj);
	Py_DECREF(xObj);
	Py_DECREF(yObj);
}
