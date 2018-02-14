#include "TextureRegion.h"
#include <structmember.h>
#include "Texture.h"

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
	delete self->_object; // FIXME
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyMethodDef TextureRegion_methods[] = {
	{ NULL }
};

static PyMemberDef TextureRegion_members[] = {
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
	0,
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

void TextureRegion::writeVertices(std::vector<Batch::attributeType> &vertexAttribData, int offset, GLfloat x, GLfloat y)
{
	// !TODO: replace with non-fixed values
	vertexAttribData[8 * 4 * offset + 8 * 0 + 0] = 0 + x;
	vertexAttribData[8 * 4 * offset + 8 * 0 + 1] = 0 + y;
	vertexAttribData[8 * 4 * offset + 8 * 1 + 0] = width + x;
	vertexAttribData[8 * 4 * offset + 8 * 1 + 1] = 0 + y;
	vertexAttribData[8 * 4 * offset + 8 * 2 + 0] = 0 + x;
	vertexAttribData[8 * 4 * offset + 8 * 2 + 1] = height + y;
	vertexAttribData[8 * 4 * offset + 8 * 3 + 0] = width + x;
	vertexAttribData[8 * 4 * offset + 8 * 3 + 1] = height + y;
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
