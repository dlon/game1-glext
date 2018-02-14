#include <Python.h>
#include <structmember.h>
#include "glutil.hpp"
#include <Windows.h>
#include <gl/GL.h>
#include "Texture.h"
#include "TextureRegion.h"

static PyObject* spam_setclearcolor(PyObject *self, PyObject *args)
{
	float r, g, b, a;
	if (!PyArg_ParseTuple(args, "ffff", &r, &g, &b, &a))
		return NULL;

	printf("set color: %f %f %f %f\n", r, g, b, a);
	glClearColor(r, g, b, a);

	Py_RETURN_NONE;
}

static PyObject* spam_clear(PyObject *self, PyObject *args)
{
	//puts("clearing");
	glClear(GL_COLOR_BUFFER_BIT);
	Py_RETURN_NONE;
}

static PyObject* spam_setviewport(PyObject *self, PyObject *args)
{
	int x, y;
	size_t w, h;
	if (!PyArg_ParseTuple(args, "iiII", &x, &y, &w, &h))
		return NULL;

	printf("setviewport: %d %d %u %u\n", x, y, w, h);
	glViewport(x, y, w, h);

	Py_RETURN_NONE;
}

#include "batch.hpp"

struct glrenderer_Batch {
	PyObject_HEAD
	Batch* _object;
};

static PyObject* Batch_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	glrenderer_Batch *self;
	self = (glrenderer_Batch*)type->tp_alloc(type, 0);
	if (self != NULL) {
		/*self->first = PyUnicode_FromString("");
		if (self->first == NULL) {
			Py_DECREF(self);
			return NULL;
		}*/
		self->_object = new Batch(1000);
	}
	return (PyObject*)self;
}

static int Batch_init(glrenderer_Batch *self, PyObject *args, PyObject *kwds) {
	/*size_t batchSize;
	if (!PyArg_ParseTuple(args, "I", &batchSize))
		return -1;*/
	return 0;
}

static void Batch_dealloc(glrenderer_Batch* self) {
	//Py_XDECREF(self->first);
	delete self->_object;
	// FIXME: ref fuckup
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
	// FIXME: ref count
	return PyLong_FromLong(self->_object->getBatchSize());
}

static PyMethodDef Batch_methods[] = {
	{ "begin", (PyCFunction)Batch_begin, METH_NOARGS, NULL },
	{ "flush", (PyCFunction)Batch_flush, METH_NOARGS, NULL },
	{ "end", (PyCFunction)Batch_end, METH_NOARGS, NULL },
	{ "draw", (PyCFunction)Batch_draw, METH_VARARGS | METH_KEYWORDS, NULL },
	{ NULL }
};

static PyMemberDef Batch_members[] = {
	{ NULL }
};

static PyGetSetDef Batch_getset[] = {
	{ "maxQuads",  (getter)Batch_getMaxQuads, 0, 0, 0 },
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

static PyMethodDef methods[] = {
	{ "setviewport", spam_setviewport, METH_VARARGS },
	{ "setclearcolor", spam_setclearcolor, METH_VARARGS },
	{ "clear", spam_clear, METH_VARARGS },
	{ "init", glrenderer_init, METH_VARARGS },
	0
};
// keyword arguments: use METH_KEYWORDS

static struct PyModuleDef glextModule = {
	PyModuleDef_HEAD_INIT,
	"glrenderer",   /* name of module */
	NULL, /* module documentation, may be NULL */
	-1,       /* size of per-interpreter state of the module,
			  or -1 if the module keeps state in global variables. */
	methods
};

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

	m = PyModule_Create(&glextModule);
	if (m == NULL)
		return NULL;

	//SpamError = PyErr_NewException("spam.error", NULL, NULL);
	//Py_INCREF(SpamError);
	//PyModule_AddObject(m, "error", SpamError);

	//loadGLFunctions(); // TODO: fix using fake WGL context

	//glrenderer_BatchType.tp_new = PyType_GenericNew;
	PyModule_AddObject(m, "Batch", (PyObject *)&glrenderer_BatchType);
	PyModule_AddObject(m, "Texture", (PyObject *)&glrenderer_TextureType);
	PyModule_AddObject(m, "TextureRegion", (PyObject *)&glrenderer_TextureRegionType);

	return m;
}
