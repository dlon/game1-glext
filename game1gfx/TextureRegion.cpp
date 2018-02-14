#include "TextureRegion.h"
#include <structmember.h>
#include "Texture.h"
#include <cmath>

static PyObject* TextureRegion_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	glrenderer_TextureRegion *self;
	self = (glrenderer_TextureRegion*)type->tp_alloc(type, 0);
	if (self != NULL) {
	}
	return (PyObject*)self;
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

	self->_object = new TextureRegion(
		*texture->textureObject,
		subX, subY,
		subWidth, subHeight
	);

	// FIXME: memory leaks?

	return 0;
}

static void TextureRegion_dealloc(glrenderer_TextureRegion* self) {
	delete self->_object; // FIXME: ref count?
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *
TextureRegion_getWidth(glrenderer_TextureRegion *self, void *closure)
{
	// FIXME: ref count
	return PyFloat_FromDouble(self->_object->width);
}

static PyObject *
TextureRegion_getHeight(glrenderer_TextureRegion *self, void *closure)
{
	// FIXME: ref count
	return PyFloat_FromDouble(self->_object->height);
}

static PyObject *
TextureRegion_getColor(glrenderer_TextureRegion *self, void *closure)
{
	// FIXME: ref count
	// FIXME: return a shared tuple
	return PyTuple_Pack(
		4,
		PyLong_FromLong(self->_object->color[0]),
		PyLong_FromLong(self->_object->color[1]),
		PyLong_FromLong(self->_object->color[2]),
		PyLong_FromLong(self->_object->color[3]),
		PyLong_FromLong(self->_object->color[4])
	);
}

static int
TextureRegion_setColor(glrenderer_TextureRegion *self, PyObject *args, void *closure)
{
	// FIXME: ref count?
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
TextureRegion_setWidth(glrenderer_TextureRegion *self, PyObject *args, void *closure)
{
	// FIXME: ref count?
	float width;
	if (!PyArg_Parse(args, "f", &width))
		return -1;
	self->_object->width = width;
	return 0;
}

static int
TextureRegion_setHeight(glrenderer_TextureRegion *self, PyObject *args, void *closure)
{
	// FIXME: ref count?
	float height;
	if (!PyArg_Parse(args, "f", &height))
		return -1;
	self->_object->height = height;
	return 0;
}

static PyGetSetDef TextureRegion_getset[] = {
	{ "width",  (getter)TextureRegion_getWidth, (setter)TextureRegion_setWidth, 0, 0 },
	{ "height",  (getter)TextureRegion_getHeight, (setter)TextureRegion_setHeight, 0, 0 },
	{ "color",  (getter)TextureRegion_getColor, (setter)TextureRegion_setColor, 0, 0 },
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
	0, //TextureRegion_methods,
	0, //TextureRegion_members,
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
	relativeVertices[2] = tw;
	relativeVertices[3] = 0;
	relativeVertices[4] = 0;
	relativeVertices[5] = th;
	relativeVertices[6] = tw;
	relativeVertices[7] = th;

	float c = cos(angle);
	float s = sin(angle);
	float scaleX = getScaleX();
	float scaleY = getScaleY();

	for (int i = 0; i < sizeof(relativeVertices) / sizeof(GLfloat); i += 2) {
		float ox = relativeVertices[i + 0] - origin[0];
		float oy = relativeVertices[i + 1] - origin[1];
		relativeVertices[i + 0] = (((ox * c + oy * s)) + origin[0]) * scaleX;
		relativeVertices[i + 1] = (((oy * c - ox * s)) + origin[1]) * scaleY;
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

void TextureRegion::writeTexCoords(std::vector<Batch::attributeType> &vertexAttribData, int offset, GLfloat x, GLfloat y)
{
	float normalizedTexCoords[] = {
		tx / texture.width, ty / texture.height,
		(tx + tw) / texture.width, ty / texture.height,
		(tx) / texture.width, (ty + th) / texture.height,
		(tx + tw) / texture.width, (ty + th) / texture.height,
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
