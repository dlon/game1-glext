#include <Python.h>
#include <structmember.h>
#include "Texture.h"

static PyObject* Texture_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	glrenderer_Texture *self;
	self = (glrenderer_Texture*)type->tp_alloc(type, 0);
	if (self != NULL) {
	}
	return (PyObject*)self;
}

static int Texture_init(glrenderer_Texture *self, PyObject *args, PyObject *kwds) {
	unsigned const char *data = NULL;
	Py_ssize_t dataLen;
	unsigned int width;
	unsigned int height;

	static char *kwlist[] = { "width", "height", "data", 0 };
	
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "IIs#", kwlist, &width, &height, &data, &dataLen))
		return -1;

	printf("w/h: %d %d\n", width, height);
	//self->textureObject->loadData(width, height, data);
	self->textureObject = new Texture(width, height, data);

	// FIXME: memory leaks?

	return 0;
}

static void Texture_dealloc(glrenderer_Texture* self) {
	//Py_XDECREF(self->first);
	//delete self->textureObject; // FIXME: memory leak
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *
Texture_id(glrenderer_Texture *self)
{
	return PyLong_FromSize_t(self->textureObject->texture);
}

static PyObject *
Texture_getWidth(glrenderer_Texture *self)
{
	return PyLong_FromSize_t(self->textureObject->width);
}

static PyObject *
Texture_getHeight(glrenderer_Texture *self)
{
	return PyLong_FromSize_t(self->textureObject->height);
}

static PyMethodDef Texture_methods[] = {
	{ "id", (PyCFunction)Texture_id, METH_NOARGS, NULL },
	{ "width", (PyCFunction)Texture_getWidth, METH_NOARGS, NULL },
	{ "height", (PyCFunction)Texture_getHeight, METH_NOARGS, NULL },
	{ NULL }
};

static PyMemberDef Texture_members[] = {
	{ NULL }
};

PyTypeObject glrenderer_TextureType = {
	PyObject_HEAD_INIT(NULL, 0)
	"glrenderer.Texture",        /*tp_name*/
	sizeof(glrenderer_Texture),/*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)Texture_dealloc, /*tp_dealloc*/
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
	0,//Texture_methods, /*methods*/
	Texture_members,
	0,
	0,
	0,
	0,
	0,
	0,
	(initproc)Texture_init,
	0,
	Texture_new
};


void Texture::loadData(int width, int height, const unsigned char *data)
{
	// TODO: exception if it's already loaded

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		width,
		height,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		data
	);

	glTexParameterf(
		GL_TEXTURE_2D,
		GL_TEXTURE_MIN_FILTER,
		GL_NEAREST
	);
	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_MAG_FILTER,
		GL_NEAREST
	);

	this->width = width;
	this->height = height;
}

Texture::Texture(int width, int height, const unsigned char *data)
{
	loadData(width, height, data);
}


Texture::~Texture()
{
	glDeleteTextures(1, &texture);
}
