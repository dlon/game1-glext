#pragma once

#include <pyUtil.h>

extern PyTypeObject glrenderer_CArrayType;

// wrap a C array
PyObject*
glrenderer_CArray_New(void *arr, const char *format, int itemsize, int ndim, Py_ssize_t *shape);
