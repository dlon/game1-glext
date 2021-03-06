#include <Python.h>
#include <structmember.h>
#include "Texture.h"

static PyObject* Texture_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	glrenderer_Texture *self;
	self = (glrenderer_Texture*)type->tp_alloc(type, 0);
	if (self != NULL)
		return (PyObject*)self;
	return NULL;
}

static int Texture_init(glrenderer_Texture *self, PyObject *args, PyObject *kwds) {
	unsigned int width;
	unsigned int height;
	Py_buffer dataBuffer;

	static char *kwlist[] = { "width", "height", "data", 0 };
	
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "IIz*", kwlist, &width, &height, &dataBuffer))
		return -1;

	self->textureObject = new Texture(width, height, (const unsigned char*)dataBuffer.buf);
	self->owner = 1;

	PyBuffer_Release(&dataBuffer);

	return 0;
}

static void Texture_dealloc(glrenderer_Texture* self) {
	if (self->owner)
		delete self->textureObject;
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
	{ NULL }
};

static PyGetSetDef Texture_getset[] = {
	{ "texture", (getter)Texture_id, 0, 0, 0 },
	{ "width", (getter)Texture_getWidth, 0, 0, 0 },
	{ "height", (getter)Texture_getHeight, 0, 0, 0 },
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
	0,//Texture_methods,
	0,//Texture_members,
	Texture_getset,
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

	glEnable(GL_TEXTURE_2D);

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

	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_MIN_FILTER,
		GL_NEAREST
	);
	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_MAG_FILTER,
		GL_NEAREST
	);

	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_WRAP_S,
		GL_CLAMP_TO_EDGE
	);
	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_WRAP_T,
		GL_CLAMP_TO_EDGE
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
