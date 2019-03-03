#include "DefaultEntityRenderer.h"
#include <pyUtil.h>
#include "../TextureRegion.h"
#include <structmember.h>
#include <stddef.h>


int loadSpriteDictionary(PyObject *dict)
{
	PyObject *gfxMod = PyImport_ImportModule("gfx");
	if (!gfxMod)
		return -1;
	PyObject *glMod = PyObject_GetAttrString(gfxMod, "gl");
	Py_DECREF(gfxMod);
	if (!glMod)
		return -1;
	PyObject *loadDict = PyObject_GetAttrString(glMod, "loadDictionary");
	Py_DECREF(glMod);
	if (!loadDict)
		return -1;

	PyObject * ret = PyObject_CallFunctionObjArgs(loadDict, dict, NULL);
	Py_DECREF(loadDict);
	if (!ret)
		return -1;
	Py_DECREF(ret);

	return 0;
}

typedef struct {
	PyObject_HEAD
	PyObject *entity;
	PyObject *rendererMap;
} DefaultEntityRendererObject;

static int DefaultEntityRenderer_init(DefaultEntityRendererObject *self, PyObject *args, PyObject *kwds)
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

		PyObject *spritesCopy = PyObject_DeepCopy(sprites);
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

	// TODO: particles
	/*
		from . import particles
		self.particleRenderers = {}
		if hasattr(entity, 'particleTypes'):
			for var, cls in entity.particleTypes.items():
				self.particleRenderers[var] =\
					particles.ParticleGroupRenderer(cls)
	*/

	return 0;
}

static void DefaultEntityRenderer_dealloc(DefaultEntityRendererObject *self)
{
	Py_XDECREF(self->rendererMap);
	Py_XDECREF(self->entity);
	Py_TYPE(self)->tp_free((PyObject *)self);
}

static Batch* getRendererBatch(PyObject *renderer)
{
	PyObject *batchObj = PyObject_GetAttrString(renderer, "batch");
	if (!batchObj) {
		return NULL;
	}
	Batch *batch = ((glrenderer_Batch*)batchObj)->_object;
	Py_DECREF(batch);
	return batch;
}

static PyObject* getEntityRegion(PyObject *entity)
{
	PyObject *sprites = PyObject_GetAttrString(entity, "_sprites");
	if (PyObject_Not(sprites)) {
		Py_DECREF(sprites);
		Py_RETURN_NONE;
	}

	PyObject *spriteName = PyObject_GetAttrString(entity, "sprite");
	if (!spriteName) {
		PyErr_Clear();
		spriteName = PyUnicode_FromString("default");
	}

	PyObject *region = PyDict_GetItem(sprites, spriteName);
	Py_DECREF(spriteName);
	Py_DECREF(sprites);
	if (!region) {
		// TODO: throw GraphicsError
		PyErr_Format(PyExc_SystemError, "%R has no sprite '%S'.", entity, spriteName);
		return NULL;
	}

	return region;
}

static TextureRegion* prepareEntityRegion(PyObject *entity, PyObject *region)
{
	PyObject *subRegion = NULL;
	if (region == Py_None)
		return NULL;

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

	TextureRegion *tr = ((glrenderer_TextureRegion*)region)->_object;
	Py_XDECREF(subRegion);

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

	return tr;
}

static int getEntityPos(PyObject *entity, float *x, float *y)
{
	PyObject *xObj = PyObject_GetAttrString(entity, "x");
	if (!xObj)
		return -1;
	PyObject *yObj = PyObject_GetAttrString(entity, "y");
	if (!yObj) {
		Py_DECREF(xObj);
		return -1;
	}
	*x = PyFloat_AsDouble(xObj);
	*y = PyFloat_AsDouble(yObj);
	Py_DECREF(xObj);
	Py_DECREF(yObj);
	return 0;
}

static PyObject* DefaultEntityRenderer_drawSelf(DefaultEntityRendererObject *self, PyObject *renderer)
{
	// TODO: "visible"
	// TODO: parallax
	// TODO: imageOffset

	float x, y;
	if (getEntityPos(self->entity, &x, &y))
		return NULL;

	PyObject *regionObj = getEntityRegion(self->entity);
	if (!regionObj)
		return NULL;
	TextureRegion *tr = prepareEntityRegion(self->entity, regionObj);
	if (!tr)
		Py_RETURN_NONE;
	
	Batch *batch = getRendererBatch(renderer);
	batch->draw(*tr, x, y);

	Py_RETURN_NONE;

	/*
		if not getattr(self.entity, 'visible', True):
			return
		tr = self._prepareRegion()
		if tr:
			offset = getattr(self.entity, 'imageOffset', (0, 0))
			parallax = getattr(self.entity, 'parallax', (1, 1))
			if parallax != (1, 1):
				renderer.batch.draw(
					tr,
					self.entity.x + offset[0] +
						renderer.map.camera.x -
						parallax[0] * renderer.map.camera.x,
					self.entity.y + offset[1] +
						renderer.map.camera.y -
						parallax[1] * renderer.map.camera.y,
				)
			else:
				// we're only drawing this at the moment
	*/
}

static PyObject* DefaultEntityRenderer_draw(DefaultEntityRendererObject *self, PyObject *renderer)
{
	return DefaultEntityRenderer_drawSelf(self, renderer);

	// TODO: renderOrder
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
	NULL
};

PyMemberDef DefaultEntityRenderer_members[] = {
	{ "entity", T_OBJECT, offsetof(DefaultEntityRendererObject, entity), 0, 0 },
	{ "rendererMap", T_OBJECT, offsetof(DefaultEntityRendererObject, rendererMap), 0, 0 },
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
	(initproc)DefaultEntityRenderer_init, // tp_init
	0, // tp_alloc
	DefaultEntityRenderer_new, // tp_new
};

/*

class DefaultEntityRenderer:
	def __init__(self, entity):
		if hasattr(entity, 'sprites'):
			type(entity)._sprites = copy.deepcopy(entity.sprites)
			try:
				gfx.gl.loadDictionary(entity._sprites)
			except Exception as e:
				raise gfx.GraphicsError(
					'{}: couldn\'t load sprites dict'.format(entity)
				)
		else:
			entity._sprites = {}
		self.entity = entity
		self.rendererMap = {}

		if not hasattr(entity, 'x'):
			entity.x = 0
		if not hasattr(entity, 'y'):
			entity.y = 0

		from . import particles
		self.particleRenderers = {}
		if hasattr(entity, 'particleTypes'):
			for var, cls in entity.particleTypes.items():
				self.particleRenderers[var] =\
					particles.ParticleGroupRenderer(cls)

	def _prepareRegion(self):
		return self._prepareGivenRegion(self._getRegion())

	def _prepareSurface(self):
		return self._prepareGivenRegion(self._getRegion())

	def _getOrCreateChildRenderer(self, renderer, c):
		r = self.rendererMap.get(c)
		if not r:
			renderer.batch.end()
			self.rendererMap[c] = \
				renderer.createEntityRenderer(
					c
				)
			renderer.batch.begin()
			r = self.rendererMap[c]
		return r

	def drawChild(self, renderer, child):
		try:
			self.rendererMap.get(child)
		except TypeError:
			children = child
		else:
			children = [child]
		for c in children:
			r = self._getOrCreateChildRenderer(
				renderer,
				c
			)
			r.draw(renderer)

	@property
	def depth(self): return self.entity.depth

	@property
	def parallax(self): return self.entity.parallax

*/

