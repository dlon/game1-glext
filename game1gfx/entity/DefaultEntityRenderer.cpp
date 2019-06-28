#include "DefaultEntityRenderer.h"
#include <pyUtil.h>
#include "../TextureRegion.h"
#include <structmember.h>
#include <stddef.h>
#include "../entity.h"
#include "../gfx.h"


extern PyObject* entity_glMod;
extern PyObject* entity_glModLoadDict;
extern PyObject* entity_deepcopy;

static PyObject *ParticleGroupRenderer = nullptr;

int DefaultEntityRenderer_init()
{
	// TODO: obtain particle group renderer here somehow
	return 0;
}

int DefaultEntityRenderer_cleanup()
{
	if (ParticleGroupRenderer) {
		Py_DECREF(ParticleGroupRenderer);
		ParticleGroupRenderer = nullptr;
	}
	return 0;
}

int loadSpriteDictionary(PyObject *dict)
{
	PyObject * ret = PyObject_CallFunctionObjArgs(entity_glModLoadDict, dict, NULL);
	if (!ret)
		return -1;
	Py_DECREF(ret);

	return 0;
}

typedef struct {
	PyObject_HEAD
	PyObject *entity;
	PyObject *rendererMap;
	PyObject *particleRenderers;
} DefaultEntityRendererObject;

static int _DefaultEntityRenderer_init(DefaultEntityRendererObject *self, PyObject *args, PyObject *kwds)
{
	char* keywords[] = {
		"entity",
		0
	};

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", keywords, &self->entity))
		return -1;
	Py_INCREF(self->entity);

	if (PyObject_HasAttrString(self->entity, "sprites")) {
		PyObject *sprites = PyObject_GetAttrString(self->entity, "sprites");

		PyObject *spritesCopy = PyObject_DeepCopy(sprites, entity_deepcopy);
		loadSpriteDictionary(spritesCopy);

		PyObject *entityType = PyObject_Type(self->entity);
		PyObject_SetAttrString(entityType, "_sprites", spritesCopy);
		Py_DECREF(spritesCopy);
		Py_DECREF(entityType);
	}
	else {
		PyObject *emptyDict = PyDict_New();
		PyObject_SetAttrString(self->entity, "_sprites", emptyDict);
		Py_DECREF(emptyDict);
	}

	self->rendererMap = PyDict_New();

	PyObject *temp;
	if (!PyObject_HasAttrString(self->entity, "x")) {
		temp = PyFloat_FromDouble(0);
		PyObject_SetAttrString(self->entity, "x", temp);
		Py_DECREF(temp);
	}
	if (!PyObject_HasAttrString(self->entity, "y")) {
		temp = PyFloat_FromDouble(0);
		PyObject_SetAttrString(self->entity, "y", temp);
		Py_DECREF(temp);
	}

	self->particleRenderers = PyDict_New();
	if (PyObject_HasAttrString(self->entity, "particleTypes")) {
		/*
		for var, cls in entity.particleTypes.items():
			self.particleRenderers[var] =\
				particles.ParticleGroupRenderer(cls)
		*/
		PyObject *particleTypes = PyObject_GetAttrString(self->entity, "particleTypes");
		
		PyObject *var, *cls;
		Py_ssize_t pos = 0;

		if (!ParticleGroupRenderer) {
			// TODO: move to module init (fails currently)
			// TODO: do a relative import (of entity mod?)
			PyObject *entityMod = PyImport_ImportModule("gfx.entity");
			if (!entityMod)
				return -1;
			PyObject *particlesMod = PyObject_GetAttrString(entityMod, "particles");
			Py_DECREF(entityMod);
			if (!particlesMod)
				return -1;
			ParticleGroupRenderer = PyObject_GetAttrString(particlesMod, "ParticleGroupRenderer");
			Py_DECREF(particlesMod);
			if (!ParticleGroupRenderer) {
				return -1;
			}
		}

		while (PyDict_Next(particleTypes, &pos, &var, &cls)) {
			PyObject *particleRenderer = PyObject_CallFunctionObjArgs(ParticleGroupRenderer, cls, NULL);
			if (!particleRenderer ||
				PyDict_SetItem(self->particleRenderers, var, particleRenderer)) {
				Py_XDECREF(particleRenderer);
				return NULL;
			}
			Py_DECREF(particleRenderer);
		}
	}

	return 0;
}

static void DefaultEntityRenderer_dealloc(DefaultEntityRendererObject *self)
{
	Py_XDECREF(self->particleRenderers);
	Py_XDECREF(self->rendererMap);
	Py_XDECREF(self->entity);
	Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject* prepareEntityRegionObj(PyObject *entity, PyObject *region)
{
	PyObject *subRegion = NULL;
	if (region == Py_None)
		Py_RETURN_NONE;

	if (PySequence_Check(region)) {
		int frames = PyObject_Length(region);
		int imageIndex;
		PyObject *imageIndexObj = PyObject_GetAttrString(entity, "imageIndex");
		if (imageIndexObj)
			imageIndex = ((int)PyFloat_AsDouble(imageIndexObj) % frames);
		else {
			PyErr_Clear();
			imageIndex = 0;
		}
		subRegion = PySequence_GetItem(region, imageIndex);
		region = subRegion;
	}
	else {
		Py_INCREF(region);
	}

	TextureRegion *tr = ((glrenderer_TextureRegion*)region)->_object;

	tr->angle = PyObject_GetFloatAttribute(entity, "angle", 0.f);
	tr->setScaleX(PyObject_GetFloatAttribute(entity, "scaleX", 1.f));
	tr->setScaleY(PyObject_GetFloatAttribute(entity, "scaleY", 1.f));
	tr->flipX = PyObject_GetIntAttribute(entity, "flipX", 0);
	tr->flipY = PyObject_GetIntAttribute(entity, "flipY", 0);

	PyObject *colorObj = PyObject_GetAttrString(entity, "color");
	if (colorObj) {
		PyObject *r = PySequence_GetItem(colorObj, 0);
		PyObject *g = PySequence_GetItem(colorObj, 1);
		PyObject *b = PySequence_GetItem(colorObj, 2);

		tr->color[0] = PyFloat_AsDouble(r) / 255.f;
		tr->color[1] = PyFloat_AsDouble(g) / 255.f;
		tr->color[2] = PyFloat_AsDouble(b) / 255.f;

		Py_DECREF(r);
		Py_DECREF(g);
		Py_DECREF(b);
		Py_DECREF(colorObj);
	}
	else {
		PyErr_Clear();
		tr->color[0] = 1.f;
		tr->color[1] = 1.f;
		tr->color[2] = 1.f;
	}

	tr->color[3] = PyObject_GetFloatAttribute(entity, "alpha", 1.0f);

	PyObject *originObj = PyObject_GetAttrString(entity, "origin");
	if (originObj) {
		PyObject *originX = PySequence_GetItem(originObj, 0);
		PyObject *originY = PySequence_GetItem(originObj, 1);

		tr->origin[0] = PyFloat_AsDouble(originX);
		tr->origin[1] = PyFloat_AsDouble(originY);

		Py_DECREF(originX);
		Py_DECREF(originY);
		Py_DECREF(originObj);
	}
	else {
		PyErr_Clear();
		tr->origin[0] = tr->width / 2.f;
		tr->origin[1] = tr->height / 2.f;
	}

	return region;
}

static TextureRegion* prepareEntityRegion(PyObject *entity, PyObject *region)
{
	region = prepareEntityRegionObj(entity, region);
	TextureRegion *tr;
	if (region != Py_None)
		tr = ((glrenderer_TextureRegion*)region)->_object;
	else
		tr = NULL;
	Py_DECREF(region);
	return tr;
}

static PyObject *DefaultEntityRenderer_prepareGivenRegion(DefaultEntityRendererObject *self, PyObject *region)
{
	return prepareEntityRegionObj(self->entity, region);
}

static PyObject *DefaultEntityRenderer_prepareRegion(DefaultEntityRendererObject *self, PyObject *args)
{
	PyObject *regionObj = PyObject_CallMethod((PyObject*)self, "_getRegion", NULL);
	if (!regionObj)
		return NULL;
	PyObject *temp = prepareEntityRegionObj(self->entity, regionObj);
	Py_DECREF(regionObj);
	return temp;
}

static PyObject* DefaultEntityRenderer_getRegion(DefaultEntityRendererObject *self, PyObject *args)
{
	PyObject *ret = getEntityRegion(self->entity);
	Py_XINCREF(ret);
	return ret;
}

static PyObject* DefaultEntityRenderer_drawSelf(DefaultEntityRendererObject *self, PyObject *renderer)
{
	if (!PyObject_GetIntAttribute(self->entity, "visible", 1))
		Py_RETURN_NONE;

	float x, y;
	if (getEntityPos(self->entity, &x, &y))
		return NULL;

	PyObject *regionObj = PyObject_CallMethod((PyObject*)self, "_prepareRegion", NULL);
	if (!regionObj)
		return NULL;
	if (regionObj == Py_None)
		return regionObj;
	TextureRegion *tr = ((glrenderer_TextureRegion*)regionObj)->_object;
	Py_DECREF(regionObj);

	float imageOffset[2];
	getEntityImageOffset(self->entity, &imageOffset[0], &imageOffset[1]);

	float parallaxX, parallaxY;
	getEntityParallax(self->entity, &parallaxX, &parallaxY);
	Batch *batch = getRendererBatch(renderer);
	
	if (parallaxX != 1.f || parallaxY != 1.f) {
		PyObject *map = PyObject_BorrowAttributeOrNull(renderer, "map");
		PyObject *cam = PyObject_BorrowAttributeOrNull(map, "camera");
		float camX = PyObject_GetFloatAttribute(cam, "x", 0.0f);
		float camY = PyObject_GetFloatAttribute(cam, "y", 0.0f);
		
		batch->draw(
			*tr,
			x + imageOffset[0] + camX - parallaxX * camX,
			y + imageOffset[1] + camY - parallaxY * camY
		);
	}
	else {
		batch->draw(*tr, x + imageOffset[0], y + imageOffset[1]);
	}

	Py_RETURN_NONE;
}

static PyObject *getOrCreateChildRenderer(DefaultEntityRendererObject *self, PyObject *renderer, PyObject *c)
{
	PyObject *r = PyDict_GetItem(self->rendererMap, c);
	if (!r) {
		Batch *batch = getRendererBatch(renderer);
		batch->end();

		r = PyObject_CallMethod(renderer, "createEntityRenderer", "O", c);
		PyDict_SetItem(self->rendererMap, c, r);
		Py_DECREF(r);

		batch->begin();
	}
	return r;
}

static PyObject *DefaultEntityRenderer_getOrCreateChildRenderer(DefaultEntityRendererObject *self, PyObject *args)
{
	PyObject *renderer, *entity, *child;
	if (!PyArg_ParseTuple(args, "OO", &renderer, &entity))
		return NULL;
	child = getOrCreateChildRenderer(self, renderer, entity);
	if (child)
		Py_INCREF(child);
	return child;
}

static PyObject* drawChild(DefaultEntityRendererObject *self, PyObject *renderer, PyObject *childObj)
{
	//PyObject *seq;
	PyObject *iter = PyObject_GetIter(childObj);

	if (iter == NULL) {
		PyErr_Clear();

		// call single child renderer
		PyObject *c = getOrCreateChildRenderer(self, renderer, childObj);
		if (!c)
			return NULL;
		PyObject *result = PyObject_CallMethod(c, "draw", "O", renderer);
		if (!result)
			return NULL;
	}
	else {
		// call all child renderers
		PyObject *item;
		while (item = PyIter_Next(iter)) {
			PyObject *result;
			PyObject *cr = getOrCreateChildRenderer(self, renderer, item);
			if (!cr ||
				!(result = PyObject_CallMethod(cr, "draw", "O", renderer))) {
				Py_DECREF(item);
				Py_DECREF(iter);
				return NULL;
			}
			Py_DECREF(result);
		}
		Py_DECREF(iter);
		if (PyErr_Occurred()) {
			return NULL;
		}
	}

	Py_RETURN_NONE;
}

static PyObject* DefaultEntityRenderer_drawChild(DefaultEntityRendererObject *self, PyObject *args)
{
	PyObject *renderer, *childObj;
	if (!PyArg_ParseTuple(args, "OO", &renderer, &childObj))
		return NULL;
	return drawChild(self, renderer, childObj);
}

static PyObject* DefaultEntityRenderer_draw(DefaultEntityRendererObject *self, PyObject *renderer)
{
	PyObject *renderOrder = PyObject_GetAttrString(self->entity, "renderOrder");
	if (!renderOrder) {
		PyErr_Clear();
		//return DefaultEntityRenderer_drawSelf(self, renderer);
		PyObject *drawSelf = PyObject_GetAttrString((PyObject*)self, "drawSelf");
		PyObject *ret = PyObject_CallFunctionObjArgs(drawSelf, renderer, NULL);
		Py_DECREF(drawSelf);
		return ret;
	}
	
	PyObject *seq = PySequence_Fast(renderOrder, "renderOrder attribute must support iteration");
	if (!seq)
		return NULL;

	size_t n = PySequence_Fast_GET_SIZE(seq);
	for (int i = 0; i < n; i++) {
		PyObject *rt = PySequence_Fast_GET_ITEM(seq, i);
		PyObject *particleRenderer;

		if (PyUnicode_CompareWithASCIIString(rt, "self") == 0) {
			//PyObject *result = DefaultEntityRenderer_drawSelf(self, renderer);
			PyObject *drawSelf = PyObject_GetAttrString((PyObject*)self, "drawSelf");
			if (!drawSelf)
				goto LOOPFAIL;
			PyObject *result = PyObject_CallFunctionObjArgs(drawSelf, renderer, NULL);
			Py_DECREF(drawSelf);
			if (!result)
				goto LOOPFAIL;
			Py_DECREF(result);
		}
		else if (particleRenderer = PyDict_GetItem(self->particleRenderers, rt)) {
			PyObject *particles = PyObject_GetAttr(self->entity, rt);
			if (!particles)
				goto LOOPFAIL;
			PyObject *result = PyObject_CallMethod(particleRenderer, "draw", "OO",
				renderer, particles);
			Py_DECREF(particles);
			if (!result)
				goto LOOPFAIL;
			Py_DECREF(result);
		}
		else {
			PyObject *crenderer = PyObject_GetAttr(self->entity, rt);
			if (!crenderer)
				goto LOOPFAIL;
			PyObject *result = drawChild(self, renderer, crenderer);
			if (!result) {
				Py_DECREF(crenderer);
				goto LOOPFAIL;
			}
			Py_DECREF(result);
			Py_DECREF(crenderer);
		}
	}

	Py_DECREF(seq);
	Py_RETURN_NONE;

LOOPFAIL:

	Py_DECREF(seq);
	return NULL;

	/*
	def draw(self, renderer):
		if not hasattr(self.entity, 'renderOrder'):
			self.drawSelf(renderer)
			return
		renderOrder = self.entity.renderOrder
		for rt in renderOrder:
			if rt == 'self':
				self.drawSelf(renderer)
			elif rt in self.particleRenderers:
				self.particleRenderers[rt].draw(
					renderer,
					#self.entity.__dict__[rt],
					getattr(self.entity, rt),
				)
			else:
				self.drawChild(
					renderer,
					getattr(self.entity, rt),
				)
	*/
}

static PyObject *DefaultEntityRenderer_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds)
{
	return subtype->tp_alloc(subtype, 0);
}

PyMethodDef DefaultEntityRenderer_methods[] = {
	{ "draw", (PyCFunction)DefaultEntityRenderer_draw, METH_O, NULL },
	{ "drawChild", (PyCFunction)DefaultEntityRenderer_drawChild, METH_VARARGS, NULL },
	{ "_getOrCreateChildRenderer", (PyCFunction)DefaultEntityRenderer_getOrCreateChildRenderer, METH_VARARGS, NULL },
	{ "_getRegion", (PyCFunction)DefaultEntityRenderer_getRegion, METH_NOARGS, NULL },
	{ "_prepareSurface", (PyCFunction)DefaultEntityRenderer_prepareRegion, METH_NOARGS, NULL },
	{ "_prepareRegion", (PyCFunction)DefaultEntityRenderer_prepareRegion, METH_NOARGS, NULL },
	{ "_prepareGivenRegion", (PyCFunction)DefaultEntityRenderer_prepareGivenRegion, METH_O, NULL },
	{ "drawSelf", (PyCFunction)DefaultEntityRenderer_drawSelf, METH_O, NULL },
	NULL
};

PyMemberDef DefaultEntityRenderer_members[] = {
	{ "entity", T_OBJECT, offsetof(DefaultEntityRendererObject, entity), 0, 0 },
	{ "rendererMap", T_OBJECT, offsetof(DefaultEntityRendererObject, rendererMap), 0, 0 },
	{ "particleRenderers", T_OBJECT, offsetof(DefaultEntityRendererObject, particleRenderers), 0, 0 },
	NULL
};

PyTypeObject DefaultEntityRendererType = {
	PyObject_HEAD_INIT(NULL)
	"DefaultEntityRenderer",
	sizeof(DefaultEntityRendererObject),
	0,										/*tp_itemsize*/
	(destructor)DefaultEntityRenderer_dealloc,		/*tp_dealloc*/
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
	DefaultEntityRenderer_methods, // tp_methods
	DefaultEntityRenderer_members, // tp_members
	0, // tp_getset
	0, // tp_base
	0, // tp_dict
	0, // tp_descr_get;
	0, // tp_descr_set
	0, // tp_dictoffset
	(initproc)_DefaultEntityRenderer_init, // tp_init
	0, // tp_alloc
	DefaultEntityRenderer_new, // tp_new
};

/*

class DefaultEntityRenderer:
	def _prepareSurface(self):
		return self._prepareGivenRegion(self._getRegion())

	@property
	def depth(self): return self.entity.depth

	@property
	def parallax(self): return self.entity.parallax

*/

