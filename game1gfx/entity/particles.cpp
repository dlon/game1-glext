#include "particles.h"
#include "../gfx.h"
#include "../TextureRegion.h"
#include <pyUtil.h>

struct ParticleGroupRendererObject;
typedef struct ParticleGroupRendererObject;

typedef void(*RenderCallback)(ParticleGroupRendererObject *self, PyObject *renderer, PyObject *particles);

enum ParticleRenderOptions {
	PARTRENDER_DYNAMIC_TRANSFORM = 0x01,
	PARTRENDER_DYNAMIC_COLOR = 0x02,
	PARTRENDER_DYNAMIC_SPRITE = 0x04,
	PARTRENDER_DYNAMIC_MODE = 0x08
};

typedef struct ParticleGroupRendererObject {
	PyObject_HEAD
	PyObject *particleClass;
	RenderCallback draw;

	TextureRegion *region;
	unsigned int blendSrc;
	unsigned int blendDest;

	/* ParticleRenderOptions */
	int renderOptions;
} ParticleGroupRendererObject;

static void updateTransform(TextureRegion *region, PyObject *particleOrCls)
{
	region->angle = 2.0f * (float)Py_MATH_PI * fmod(PyObject_GetFloatAttribute(particleOrCls, "angle", 0.0f) / 360.0f, 360.0f);
	float size = PyObject_GetFloatAttribute(particleOrCls, "size", 1.0f);
	region->setScaleX(size);
	region->setScaleY(size);
	region->origin[0] = size * region->tw / 2.f; // TODO: tw or width?
	region->origin[1] = size * region->th / 2.f;
}

static void updateColor(TextureRegion *region, PyObject *particleOrCls)
{
	PyObject *color = PyObject_GetAttrString(particleOrCls, "color");
	if (!color) {
		PyErr_Clear();
		region->color[0] = 1.f;
		region->color[1] = 1.f;
		region->color[2] = 1.f;
	}
	else {
		PyObject *temp = PySequence_Fast(color, "color is not a sequence");
		if (!temp)
			return;
		Py_DECREF(color);
		color = temp;

		region->color[0] = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(color, 0)) / 255.f;
		region->color[1] = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(color, 1)) / 255.f;
		region->color[2] = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(color, 2)) / 255.f;

		Py_DECREF(color);
	}
	region->color[3] = PyObject_GetFloatAttribute(particleOrCls, "alpha", 1.0f);
}

static void updateMode(ParticleGroupRendererObject *self, PyObject *particleOrCls)
{
	PyObject *mode = PyObject_GetAttrString(particleOrCls, "mode");
	if (mode && PyUnicode_Check(mode) &&
		PyUnicode_CompareWithASCIIString(mode, "additive") == 0) {
		self->blendSrc = GL_ONE;
		self->blendDest = GL_ONE;
	}
	else {
		PyErr_Clear();
		self->blendSrc = GL_SRC_ALPHA;
		self->blendDest = GL_ONE_MINUS_SRC_ALPHA;
	}
	Py_XDECREF(mode);
}

static bool obtainSprite(TextureRegion **region, PyObject *particleOrCls)
{
	PyObject *spriteAttr = PyObject_GetAttrString(particleOrCls, "sprite");
	if (!spriteAttr)
		return false;
	*region = gfx_loadOrGet(spriteAttr);
	Py_DECREF(spriteAttr);
	return *region != 0;
}

static void drawParticleSprites(ParticleGroupRendererObject *self, PyObject *renderer, PyObject *particles)
{
	int renderOptions = self->renderOptions;
	
	Batch *batch = getRendererBatch(renderer);
	GLenum oldBlendMode[2];
	batch->getBlendMode(oldBlendMode);
	batch->setBlendMode(self->blendSrc, self->blendDest);

	TextureRegion *region;
	float hw, hh;
	if (!(renderOptions & PARTRENDER_DYNAMIC_SPRITE)) {
		region = self->region;
		hw = region->width / 2.f;
		hh = region->height / 2.f;
	}

	PyObject *iter = PyObject_GetIter(particles);
	PyObject *particle;

	while (particle = PyIter_Next(iter)) {
		if (renderOptions & PARTRENDER_DYNAMIC_SPRITE) {
			// FIXME: cache in dict & .end() + .begin()
			if (!obtainSprite(&region, particle)) {
				return;
			}
			hw = region->width / 2.f;
			hh = region->height / 2.f;
		}
		if (renderOptions & PARTRENDER_DYNAMIC_COLOR)
			updateColor(region, particle);
		if (renderOptions & PARTRENDER_DYNAMIC_TRANSFORM)
			updateTransform(region, particle);
		if (renderOptions & PARTRENDER_DYNAMIC_MODE) {
			updateMode(self, particle);
			batch->setBlendMode(self->blendSrc, self->blendDest);
		}

		float x = PyObject_GetFloatAttribute(particle, "x", 0.f);
		float y = PyObject_GetFloatAttribute(particle, "y", 0.f);
		batch->draw(*region, x - hw, y - hh);

		Py_DECREF(particle);
	}

	Py_DECREF(iter);
	//if (PyErr_Occurred())

	batch->setBlendMode(oldBlendMode[0], oldBlendMode[1]);
}

static void drawParticleCircles(ParticleGroupRendererObject *self, PyObject *renderer, PyObject *particles)
{
	Batch *batch = getRendererBatch(renderer);
	glrenderer_ShapeBatch *shapes = getShapeBatch(renderer);

	batch->end();
	ShapeBatch_begin(shapes);

	// TODO: use 'color' and 'alpha'
	// TODO: set blend 'mode'
	// TODO: smoothness setting. 20 for fire, 40 normally?
	// TODO: optimization flags

	PyObject *iter = PyObject_GetIter(particles);
	PyObject *particle;
	ShapeBatch_setColor(shapes, 1.f, 1.f, 1.f, 1.f);

	size_t smoothness = 40;

	while (particle = PyIter_Next(iter)) {
		float x = PyObject_GetFloatAttribute(particle, "x", 0.f);
		float y = PyObject_GetFloatAttribute(particle, "y", 0.f);
		float size = PyObject_GetFloatAttribute(particle, "size", 1.f);
		
		ShapeBatch_drawCircle(shapes, x, y, size, smoothness);

		Py_DECREF(particle);
	}

	Py_DECREF(iter);
	//if (PyErr_Occurred())

	ShapeBatch_end(shapes);
	batch->begin();
}

static int ParticleGroupRenderer_init(ParticleGroupRendererObject *self, PyObject *args, PyObject *kwds)
{
	char* keywords[] = {
		"particleClass",
		0
	};

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", keywords, &self->particleClass))
		return -1;

	if (PyObject_HasAttrString(self->particleClass, "sprite")) {
		self->draw = drawParticleSprites;

		PyObject *options = PyObject_GetAttrString(self->particleClass, "particleOptions");
		PyObject *dynamicOpt = options ? PyDict_GetItemString(options, "dynamic") : NULL;

		if (!options || !dynamicOpt) {
			PyErr_Clear();
			self->renderOptions = PARTRENDER_DYNAMIC_COLOR |
				PARTRENDER_DYNAMIC_SPRITE |
				PARTRENDER_DYNAMIC_TRANSFORM |
				PARTRENDER_DYNAMIC_MODE;
		}
		else {
			if (PySequence_ContainsStr(dynamicOpt, "size") ||
				PySequence_ContainsStr(dynamicOpt, "angle") ||
				PySequence_ContainsStr(dynamicOpt, "origin"))
				self->renderOptions |= PARTRENDER_DYNAMIC_TRANSFORM;
			if (PySequence_ContainsStr(dynamicOpt, "color") ||
				PySequence_ContainsStr(dynamicOpt, "alpha"))
				self->renderOptions |= PARTRENDER_DYNAMIC_COLOR;
			if (PySequence_ContainsStr(dynamicOpt, "sprite"))
				self->renderOptions |= PARTRENDER_DYNAMIC_SPRITE;
			if (PySequence_ContainsStr(dynamicOpt, "mode"))
				self->renderOptions |= PARTRENDER_DYNAMIC_MODE;
		}
		Py_XDECREF(options);

		if (!(self->renderOptions & PARTRENDER_DYNAMIC_SPRITE)) {
			if (!obtainSprite(&self->region, self->particleClass)) {
				return -1;
			}
			if (!(self->renderOptions & PARTRENDER_DYNAMIC_COLOR))
				updateColor(self->region, self->particleClass);
			if (!(self->renderOptions & PARTRENDER_DYNAMIC_TRANSFORM))
				updateTransform(self->region, self->particleClass);
		}
		if (!(self->renderOptions & PARTRENDER_DYNAMIC_MODE))
			updateMode(self, self->particleClass);
	}
	else {
		PyObject *pType = PyObject_GetAttrString(self->particleClass, "particleType");
		if (!pType)
			return -1;
		if (PyUnicode_CompareWithASCIIString(pType, "circle") == 0) {
			self->draw = drawParticleCircles;
			Py_DECREF(pType);
		}
		else {
			PyErr_SetString(PyExc_RuntimeError, "stub");
			Py_DECREF(pType);
			return -1;
		}
	}

	return 0;
}

static PyObject *ParticleGroupRenderer_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds)
{
	return subtype->tp_alloc(subtype, 0);
}

static void ParticleGroupRenderer_dealloc(PyObject *self)
{
	return Py_TYPE(self)->tp_free(self);
}

static PyObject* ParticleGroupRenderer_draw(ParticleGroupRendererObject *self, PyObject *args)
{
	PyObject *renderer, *particles;
	if (!PyArg_ParseTuple(args, "OO", &renderer, &particles))
		return NULL;

	self->draw(self, renderer, particles);
	Py_RETURN_NONE;
}

PyMethodDef ParticleGroupRenderer_methods[] = {
	{ "draw", (PyCFunction)ParticleGroupRenderer_draw, METH_VARARGS, NULL },
	NULL
};

PyTypeObject ParticleGroupRendererType = {
	PyObject_HEAD_INIT(NULL)
	"ParticleGroupRenderer",
	sizeof(ParticleGroupRendererObject),
	0,										/*tp_itemsize*/
	ParticleGroupRenderer_dealloc,			/*tp_dealloc*/
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
	0,										/*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	NULL,									/* tp_doc */
	0, // tp_traverse
	0, // tp_clear
	0, // tp_richcompare
	0, // tp_weaklistoffset
	0, // tp_iter
	0, // tp_iternex
	ParticleGroupRenderer_methods, // tp_methods
	0, // tp_members
	0, // tp_getset
	0, // tp_base
	0, // tp_dict
	0, // tp_descr_get;
	0, // tp_descr_set
	0, // tp_dictoffset
	(initproc)ParticleGroupRenderer_init, // tp_init
	0, // tp_alloc
	ParticleGroupRenderer_new, // tp_new
};
