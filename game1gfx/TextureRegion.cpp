#include "TextureRegion.h"
#include <structmember.h>
#include "Texture.h"
#include <cmath>

static PyObject* TextureRegion_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	glrenderer_TextureRegion *self;
	self = (glrenderer_TextureRegion*)type->tp_alloc(type, 0);
	if (self != NULL)
		return (PyObject*)self;
	return NULL;
}

static int TextureRegion_init(glrenderer_TextureRegion *self, PyObject *args, PyObject *kwds) {
	unsigned int subX;
	unsigned int subY;
	unsigned int subWidth;
	unsigned int subHeight;
	glrenderer_Texture* texture = NULL;

	static char *kwlist[] = { "texture", "subX", "subY", "subwidth", "subheight", 0 };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!IIII", kwlist,
		&glrenderer_TextureType, &texture,
		&subX, &subY, &subWidth, &subHeight))
		return -1;

	Py_INCREF(texture);
	self->tex = texture;
	self->owner = 1;
	self->_object = new TextureRegion(
		*texture->textureObject,
		subX, subY,
		subWidth, subHeight
	);

	return 0;
}

static void TextureRegion_dealloc(glrenderer_TextureRegion* self) {
	if (self->owner)
		delete self->_object;
	Py_XDECREF(self->tex);
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *
TextureRegion_getWidth(glrenderer_TextureRegion *self, void *closure)
{
	return PyFloat_FromDouble(self->_object->width);
}

static PyObject *
TextureRegion_getHeight(glrenderer_TextureRegion *self, void *closure)
{
	return PyFloat_FromDouble(self->_object->height);
}

static PyObject *
TextureRegion_getColor(glrenderer_TextureRegion *self, void *closure)
{
	// FIXME: return a shared tuple
	return Py_BuildValue(
		"(ffff)",
		self->_object->color[0],
		self->_object->color[1],
		self->_object->color[2],
		self->_object->color[3]
	);
}

static int
TextureRegion_setColor(glrenderer_TextureRegion *self, PyObject *args, void *closure)
{
	float r, g, b, a;
	if (!PyArg_ParseTuple(args, "ffff", &r, &g, &b, &a))
		return -1;

	self->_object->color[0] = r;
	self->_object->color[1] = g;
	self->_object->color[2] = b;
	self->_object->color[3] = a;

	return 0;
}

static int
TextureRegion_setRGB255(glrenderer_TextureRegion *self, PyObject *args, void *closure)
{
	float r, g, b;
	if (!PyArg_ParseTuple(args, "fff", &r, &g, &b))
		return -1;

	self->_object->color[0] = r / 255.0f;
	self->_object->color[1] = g / 255.0f;
	self->_object->color[2] = b / 255.0f;

	return 0;
}

static PyObject *
TextureRegion_getRGB255(glrenderer_TextureRegion *self, void *closure)
{
	return Py_BuildValue(
		"(fff)",
		self->_object->color[0] * 255.0f,
		self->_object->color[1] * 255.0f,
		self->_object->color[2] * 255.0f
	);
}

static int
TextureRegion_setAlpha(glrenderer_TextureRegion *self, PyObject *args, void *closure)
{
	float a;
	if (!PyArg_Parse(args, "f", &a))
		return -1;
	self->_object->color[3] = a;
	return 0;
}

static PyObject *
TextureRegion_getAlpha(glrenderer_TextureRegion *self, void *closure)
{
	return PyFloat_FromDouble(self->_object->color[3]);
}

static int
TextureRegion_setWidth(glrenderer_TextureRegion *self, PyObject *args, void *closure)
{
	float width;
	if (!PyArg_Parse(args, "f", &width))
		return -1;
	self->_object->width = width;
	return 0;
}

static int
TextureRegion_setHeight(glrenderer_TextureRegion *self, PyObject *args, void *closure)
{
	float height;
	if (!PyArg_Parse(args, "f", &height))
		return -1;
	self->_object->height = height;
	return 0;
}

static int
TextureRegion_setAngle(glrenderer_TextureRegion *self, PyObject *args, void *closure)
{
	float angle;
	if (!PyArg_Parse(args, "f", &angle))
		return -1;
	self->_object->setAngle(angle);
	return 0;
}

static PyObject *
TextureRegion_getAngle(glrenderer_TextureRegion *self, void *closure)
{
	return PyFloat_FromDouble(self->_object->angle);
}

static int
TextureRegion_setOrigin(glrenderer_TextureRegion *self, PyObject *args, void *closure)
{
	float x, y;
	if (!PyArg_ParseTuple(args, "ff", &x, &y))
		return -1;
	self->_object->origin[0] = x;
	self->_object->origin[1] = y;
	return 0;
}

static PyObject *
TextureRegion_getOrigin(glrenderer_TextureRegion *self, void *closure)
{
	return Py_BuildValue("ff", self->_object->origin[0], self->_object->origin[1]);
}

static int
TextureRegion_setScaleX(glrenderer_TextureRegion *self, PyObject *args, void *closure)
{
	float x;
	if (!PyArg_Parse(args, "f", &x))
		return -1;
	self->_object->setScaleX(x);
	return 0;
}

static PyObject *
TextureRegion_getScaleX(glrenderer_TextureRegion *self, void *closure)
{
	return PyFloat_FromDouble(self->_object->getScaleX());
}

static int
TextureRegion_setScaleY(glrenderer_TextureRegion *self, PyObject *args, void *closure)
{
	float y;
	if (!PyArg_Parse(args, "f", &y))
		return -1;
	self->_object->setScaleY(y);
	return 0;
}

static PyObject *
TextureRegion_getScaleY(glrenderer_TextureRegion *self, void *closure)
{
	return PyFloat_FromDouble(self->_object->getScaleY());
}

static int
TextureRegion_setFlipX(glrenderer_TextureRegion *self, PyObject *args, void *closure)
{
	int f;
	if (!PyArg_Parse(args, "p", &f))
		return -1;
	self->_object->flipX = f != 0;
	return 0;
}

static PyObject *
TextureRegion_getFlipX(glrenderer_TextureRegion *self, void *closure)
{
	return PyBool_FromLong(self->_object->flipX);
}

static int
TextureRegion_setFlipY(glrenderer_TextureRegion *self, PyObject *args, void *closure)
{
	int f;
	if (!PyArg_Parse(args, "p", &f))
		return -1;
	self->_object->flipY = f != 0;
	return 0;
}

static PyObject *
TextureRegion_getFlipY(glrenderer_TextureRegion *self, void *closure)
{
	return PyBool_FromLong(self->_object->flipY);
}

static PyObject *
TextureRegion_getSubWidth(glrenderer_TextureRegion *self, void *closure)
{
	return PyFloat_FromDouble(self->_object->tw);
}

static PyObject *
TextureRegion_getSubHeight(glrenderer_TextureRegion *self, void *closure)
{
	return PyFloat_FromDouble(self->_object->th);
}

static PyObject *
TextureRegion_getSubX(glrenderer_TextureRegion *self, void *closure)
{
	return PyFloat_FromDouble(self->_object->tx);
}

static int
TextureRegion_setSubX(glrenderer_TextureRegion *self, PyObject *args, void *closure)
{
	float tx;
	if (!PyArg_Parse(args, "f", &tx))
		return -1;
	self->_object->tx = tx;
	return 0;
}

static PyObject *
TextureRegion_getSubY(glrenderer_TextureRegion *self, void *closure)
{
	return PyFloat_FromDouble(self->_object->ty);
}

static int
TextureRegion_setSubY(glrenderer_TextureRegion *self, PyObject *args, void *closure)
{
	float ty;
	if (!PyArg_Parse(args, "f", &ty))
		return -1;
	self->_object->ty = ty;
	return 0;
}

static PyObject *
TextureRegion_getTexture(glrenderer_TextureRegion *self, void *closure)
{
	// !!!FIXME: ref count!
	// TODO: return texture python object, not ID
	return PyLong_FromSize_t(self->_object->texture.texture);
}

static PyObject *
TextureRegion_getNormalizedTexCoords(glrenderer_TextureRegion *self, void *closure)
{
	// TODO: return texture python object?
	// FIXME: store in a list
	float coords[8];
	self->_object->getNormalizedTexCoords(coords);
	return Py_BuildValue(
		"ffffffff",
		coords[0],
		coords[1],
		coords[2],
		coords[3],
		coords[4],
		coords[5],
		coords[6],
		coords[7]
	);
}

static PyGetSetDef TextureRegion_getset[] = {
	{ "width",  (getter)TextureRegion_getWidth, (setter)TextureRegion_setWidth, 0, 0 },
	{ "height",  (getter)TextureRegion_getHeight, (setter)TextureRegion_setHeight, 0, 0 },
	{ "color",  (getter)TextureRegion_getColor, (setter)TextureRegion_setColor, 0, 0 },
	{ "RGB255",  (getter)TextureRegion_getRGB255, (setter)TextureRegion_setRGB255, 0, 0 },
	{ "alpha",  (getter)TextureRegion_getAlpha, (setter)TextureRegion_setAlpha, 0, 0 },
	{ "angle",  (getter)TextureRegion_getAngle, (setter)TextureRegion_setAngle, 0, 0 },
	{ "origin",  (getter)TextureRegion_getOrigin, (setter)TextureRegion_setOrigin, 0, 0 },
	{ "subX",  (getter)TextureRegion_getSubX, (setter)TextureRegion_setSubX, 0, 0 },
	{ "subY",  (getter)TextureRegion_getSubY, (setter)TextureRegion_setSubX, 0, 0 },
	{ "subWidth",  (getter)TextureRegion_getSubWidth, 0, 0, 0 },
	{ "subHeight",  (getter)TextureRegion_getSubHeight, 0, 0, 0 },
	{ "scaleX",  (getter)TextureRegion_getScaleX, (setter)TextureRegion_setScaleX, 0, 0 },
	{ "scaleY",  (getter)TextureRegion_getScaleY, (setter)TextureRegion_setScaleY, 0, 0 },
	{ "flipX",  (getter)TextureRegion_getFlipX, (setter)TextureRegion_setFlipX, 0, 0 },
	{ "flipY",  (getter)TextureRegion_getFlipY, (setter)TextureRegion_setFlipY, 0, 0 },
	{ "texture",  (getter)TextureRegion_getTexture, 0, 0, 0 },
	{ "normalizedCoords", (getter)TextureRegion_getNormalizedTexCoords, 0, 0, 0 },
	{ NULL }
};

static PyMemberDef TextureRegion_members[] = {
	{ "glTexture", T_OBJECT_EX, offsetof(glrenderer_TextureRegion, tex), READONLY, 0 },
	{ 0 }
};

static PyObject *
TextureRegion_transform(glrenderer_TextureRegion *self, PyObject *args)
{
	float scaleX;
	float scaleY;
	float angle;
	if (!PyArg_ParseTuple(
		args, "fff",
		&angle,
		&scaleX,
		&scaleY))
		return NULL;
	self->_object->setAngle(angle);
	self->_object->setScaleX(scaleX);
	self->_object->setScaleY(scaleY);
	Py_RETURN_NONE;
}

static PyMethodDef TextureRegion_methods[] = {
	{ "transform",  (PyCFunction)TextureRegion_transform, METH_VARARGS, 0 },
	{ NULL }
};

PyTypeObject glrenderer_TextureRegionType = {
	PyObject_HEAD_INIT(NULL, 0)
	"glrenderer.TextureRegion",        /*tp_name*/
	sizeof(glrenderer_TextureRegion),/*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)TextureRegion_dealloc, /*tp_dealloc*/
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
	TextureRegion_methods,
	TextureRegion_members,
	TextureRegion_getset,
	0,
	0,
	0,
	0,
	0,
	(initproc)TextureRegion_init,
	0,
	TextureRegion_new
};


TextureRegion::TextureRegion(const Texture& texture, int subX, int subY, int subWidth, int subHeight)
	: tx(subX), ty(subY), tw(subWidth), th(subHeight), texture(texture)
{
	width = tw;
	height = th;
}


TextureRegion::~TextureRegion()
{
}

void TextureRegion::computeTransformations()
{
	relativeVertices[0] = 0;
	relativeVertices[1] = 0;
	relativeVertices[2] = width;
	relativeVertices[3] = 0;
	relativeVertices[4] = 0;
	relativeVertices[5] = height;
	relativeVertices[6] = width;
	relativeVertices[7] = height;

	float c = cos(angle);
	float s = sin(angle);

	for (int i = 0; i < sizeof(relativeVertices) / sizeof(GLfloat); i += 2) {
		float ox = relativeVertices[i + 0] - origin[0];
		float oy = relativeVertices[i + 1] - origin[1];
		relativeVertices[i + 0] = (((ox * c + oy * s)) + origin[0]);
		relativeVertices[i + 1] = (((oy * c - ox * s)) + origin[1]);
	}
}

void TextureRegion::writeVertices(std::vector<Batch::attributeType> &vertexAttribData, int offset, GLfloat x, GLfloat y)
{
	computeTransformations();

	vertexAttribData[8 * 4 * offset + 8 * 0 + 0] = relativeVertices[0] + x;
	vertexAttribData[8 * 4 * offset + 8 * 0 + 1] = relativeVertices[1] + y;
	vertexAttribData[8 * 4 * offset + 8 * 1 + 0] = relativeVertices[2] + x;
	vertexAttribData[8 * 4 * offset + 8 * 1 + 1] = relativeVertices[3] + y;
	vertexAttribData[8 * 4 * offset + 8 * 2 + 0] = relativeVertices[4] + x;
	vertexAttribData[8 * 4 * offset + 8 * 2 + 1] = relativeVertices[5] + y;
	vertexAttribData[8 * 4 * offset + 8 * 3 + 0] = relativeVertices[6] + x;
	vertexAttribData[8 * 4 * offset + 8 * 3 + 1] = relativeVertices[7] + y;
}

void TextureRegion::getNormalizedTexCoords(float ret[])
{
	float x0 = (flipX ? (tx + tw) : tx) / texture.width;
	float x1 = (flipX ? tx : (tx + tw)) / texture.width;
	float y0 = (flipY ? (ty + th) : ty) / texture.height;
	float y1 = (flipY ? ty : (ty + th)) / texture.height;

	ret[0] = x0;
	ret[1] = y0;
	ret[2] = x1;
	ret[3] = y0;
	ret[4] = x0;
	ret[5] = y1;
	ret[6] = x1;
	ret[7] = y1;
}

void TextureRegion::writeTexCoords(std::vector<Batch::attributeType> &vertexAttribData, int offset, GLfloat x, GLfloat y)
{
	float x0 = (flipX ? (tx + tw) : tx) / texture.width;
	float x1 = (flipX ? tx : (tx + tw)) / texture.width;
	float y0 = (flipY ? (ty + th) : ty) / texture.height;
	float y1 = (flipY ? ty : (ty + th)) / texture.height;
	
	float normalizedTexCoords[] = {
		x0, y0,
		x1, y0,
		x0, y1,
		x1, y1,
	};

	vertexAttribData[8 * 4 * offset + 8 * 0 + 2] = normalizedTexCoords[0];
	vertexAttribData[8 * 4 * offset + 8 * 0 + 3] = normalizedTexCoords[1];
	vertexAttribData[8 * 4 * offset + 8 * 1 + 2] = normalizedTexCoords[2];
	vertexAttribData[8 * 4 * offset + 8 * 1 + 3] = normalizedTexCoords[3];
	vertexAttribData[8 * 4 * offset + 8 * 2 + 2] = normalizedTexCoords[4];
	vertexAttribData[8 * 4 * offset + 8 * 2 + 3] = normalizedTexCoords[5];
	vertexAttribData[8 * 4 * offset + 8 * 3 + 2] = normalizedTexCoords[6];
	vertexAttribData[8 * 4 * offset + 8 * 3 + 3] = normalizedTexCoords[7];
}

void TextureRegion::writeColors(std::vector<Batch::attributeType> &vertexAttribData, int offset, GLfloat x, GLfloat y)
{
	for (int i = 0; i < 4; i++) {
		vertexAttribData[8 * 4 * offset + 8 * i + 4] = color[0];
		vertexAttribData[8 * 4 * offset + 8 * i + 5] = color[1];
		vertexAttribData[8 * 4 * offset + 8 * i + 6] = color[2];
		vertexAttribData[8 * 4 * offset + 8 * i + 7] = color[3];
	}
}

void TextureRegion::updateArray(std::vector<Batch::attributeType> &vertexAttribData, int objectIndex, GLfloat x, GLfloat y)
{
	writeVertices(vertexAttribData, objectIndex, x, y);
	writeTexCoords(vertexAttribData, objectIndex, x, y);
	writeColors(vertexAttribData, objectIndex, x, y);
}
