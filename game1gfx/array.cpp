// TODO: 2D array. use buffer protocol
// request a buffer via PyObject_GetBuffer

#include <pyUtil.h>
#include <structmember.h>

typedef struct {
	PyObject_HEAD
	void *data;
	int ndim;
	char format[2];
	Py_ssize_t itemsize;
	Py_ssize_t *shape;
} CArrayObject;

static void CArray_dealloc(CArrayObject *self)
{
	delete[] self->shape;
	//delete[] self->strides;
	//Py_TYPE(self)->tp_free((PyObject *)self);
	PyObject_Del(self);
}

static int CArray_getbuf(CArrayObject *self, Py_buffer *view, int flags)
{
	view->buf = self->data;

	// len = itemsize * product(shape)
	view->len = 0;
	for (int i = 0; i < self->ndim; i++) {
		view->len += self->shape[i] * self->itemsize;
	}

	Py_INCREF(self);
	view->obj = (PyObject*)self;
	view->readonly = 0;
	view->itemsize = self->itemsize;
	view->format = self->format;
	view->ndim = self->ndim;
	view->shape = self->shape;
	view->strides = nullptr;
	view->suboffsets = nullptr;
	view->internal = nullptr;

	// TODO: handle flags

	return 0;
}

static PyBufferProcs CArrayBufProcs = {
	(getbufferproc)CArray_getbuf,
	nullptr // CArray_releasebuf
};

PyTypeObject glrenderer_CArrayType = {
	PyObject_HEAD_INIT(NULL)
	"CArray",
	sizeof(CArrayObject),
	0,										/*tp_itemsize*/
	(destructor)CArray_dealloc,				/*tp_dealloc*/
	0,										/*tp_print*/
	0,										/*tp_getattr*/
	0,										/*tp_setattr*/
	0,										/*tp_compare*/
	0,										/*tp_repr*/
	0,										/*tp_as_number*/
	0,										/*tp_as_sequence*/
	0,										/*tp_as_mapping*/
	0,										/*tp_hash */
	0,										/*tp_call*/
	0,										/*tp_str*/
	0,										/*tp_getattro*/
	0,										/*tp_setattro*/
	&CArrayBufProcs,						/*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,
	NULL,									/* tp_doc */
	0, // tp_traverse
	0, // tp_clear
	0, // tp_richcompare
	0, // tp_weaklistoffset
	0, // tp_iter
	0, // tp_iternex
	0, // tp_methods
	0, // tp_members
	0, // tp_getset
	0, // tp_base
	0, // tp_dict
	0, // tp_descr_get;
	0, // tp_descr_set
	0, // tp_dictoffset
	0, // tp_init
	0, // tp_alloc
	0, // tp_new
};

PyObject*
glrenderer_CArray_New(
	void *arr, const char *format, int itemsize, int ndim, Py_ssize_t *shape
)
{
	if (!format[0] || format[1]) {
		PyErr_SetString(PyExc_TypeError,
			"format should be exactly one letter");
	}

	CArrayObject *obj =
		PyObject_New(CArrayObject, &glrenderer_CArrayType);
	if (!obj)
		return nullptr;

	obj->data = arr;
	obj->itemsize = itemsize;
	obj->ndim = ndim;

	obj->format[0] = format[0];
	obj->format[1] = format[1];

	// shape: ndim elements specifying size of each dimension
	obj->shape = new Py_ssize_t[ndim];
	memcpy(obj->shape, shape, sizeof(Py_ssize_t) * ndim);

	return (PyObject*)obj;
}

#if 0
int
glrenderer_CArray(
	void *arr, int itemsize, int ndim, Py_ssize_t *shape,
	Py_buffer *view
)
{
	CArrayObject *obj =
		(CArrayObject*)PyObject_CallObject((PyObject*)&glrenderer_CArrayType, nullptr);
	if (!obj)
		return -1;

	//int ret = PyObject_GetBuffer(obj, view, flags);
	//Py_DECREF(obj);
	//return ret;

	return 0;
}

int glrenderer_CArray_destroy(Py_buffer *view)
{
	//PyBuffer_Release(view);
	Py_DECREF(view->obj);
	return 0;
}
#endif
