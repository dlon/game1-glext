#include <Python.h>
#include "glutil.hpp"
#include <Windows.h>
#include <gl/GL.h>

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

#include "batch.hpp"

PyMODINIT_FUNC
PyInit_glrenderer(void)
{
	PyObject *m;

	m = PyModule_Create(&glextModule);
	if (m == NULL)
		return NULL;

	//SpamError = PyErr_NewException("spam.error", NULL, NULL);
	//Py_INCREF(SpamError);
	//PyModule_AddObject(m, "error", SpamError);

	loadGLFunctions();

	return m;
}
