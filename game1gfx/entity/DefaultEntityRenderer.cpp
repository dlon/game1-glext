#include "DefaultEntityRenderer.h"
#include <Python.h>
#include <pyUtil.h>

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

void init(PyObject *entity) {
	if (PyObject_HasAttrString(entity, "sprites")) {
		PyObject *sprites = PyObject_GetAttrString(entity, "sprites");

		PyObject *spritesCopy = PyObject_DeepCopy(sprites);
		loadSpriteDictionary(spritesCopy);

		PyObject *entityType = PyObject_Type(entity);
		PyObject_SetAttrString(entityType, "_sprites", sprites);
		Py_DECREF(spritesCopy);
		Py_DECREF(entityType);
	}
	else {
		PyObject *emptyDict = PyDict_New();
		PyObject_SetAttrString(entity, "_sprites", emptyDict);
		Py_DECREF(emptyDict);
	}
}

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

	def _getRegion(self):
		if not self.entity._sprites:
			return
		sprite = getattr(self.entity, 'sprite', 'default')
		try:
			surface = self.entity._sprites[sprite]
		except KeyError:
			raise gfx.GraphicsError('{}: undefined image'.format(
				self.entity
			))
		return surface

	def _prepareGivenRegion(self, surface):
		if not surface:
			return
		if not isinstance(surface, gfx.gl.GLTextureRegion):
			surface = surface[int(getattr(self.entity, 'imageIndex', 0) % len(surface))]
		surface.transform(
			getattr(self.entity, 'angle', 0),
			getattr(self.entity, 'scaleX', 1),
			getattr(self.entity, 'scaleY', 1),
		)
		surface.origin = getattr(self.entity, 'origin', (surface.width/2, surface.height/2))
		surface.flipX = getattr(self.entity, 'flipX', False)
		surface.flipY = getattr(self.entity, 'flipY', False)
		surface.RGB255 = getattr(self.entity, 'color', (255, 255, 255))
		surface.alpha = getattr(self.entity, 'alpha', 1)
		return surface

	def _prepareRegion(self):
		return self._prepareGivenRegion(self._getRegion())

	def _prepareSurface(self):
		return self._prepareGivenRegion(self._getRegion())

	def drawSelf(self, renderer):
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
				renderer.batch.draw(
					tr,
					self.entity.x + offset[0],
					self.entity.y + offset[1],
				)

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

	@property
	def depth(self): return self.entity.depth

	@property
	def parallax(self): return self.entity.parallax

*/

