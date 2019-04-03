#include "gfx.h"
#include "Texture.h"


TextureRegion* gfx_loadOrGet(PyObject *obj)
{
	PyObject *glMod = PyImport_ImportModule("gfx.gl");
	if (!glMod)
		return NULL;
	PyObject *loadFunc = PyObject_GetAttrString(glMod, "loadFile");
	Py_DECREF(glMod);
	if (!loadFunc)
		return NULL;

	PyObject *img = PyObject_CallFunction(loadFunc, "O", obj);
	Py_DECREF(loadFunc);
	if (!img)
		return NULL;

	if (!PyObject_IsInstance(img, (PyObject*)&glrenderer_TextureRegionType)) {
		return NULL;
	}

	// FIXME: handle this
	// force manual deallocation
	((glrenderer_TextureRegion*)img)->tex->owner = 0;
	((glrenderer_TextureRegion*)img)->owner = 0;

	TextureRegion *region = ((glrenderer_TextureRegion*)img)->_object;

	Py_DECREF(img);
	return region;
}

Batch* getRendererBatch(PyObject *renderer)
{
	PyObject *batchObj = PyObject_GetAttrString(renderer, "batch");
	if (!batchObj) {
		return NULL;
	}
	Batch *batch = ((glrenderer_Batch*)batchObj)->_object;
	Py_DECREF(batchObj);
	return batch;
}

glrenderer_ShapeBatch* getShapeBatch(PyObject *renderer)
{
	glrenderer_ShapeBatch *batch =
		(glrenderer_ShapeBatch*)PyObject_GetAttrString(renderer, "shapeBatch");
	Py_XDECREF((PyObject*)batch);
	return batch;
}
