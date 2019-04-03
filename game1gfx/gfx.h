#pragma once

#include "TextureRegion.h"
#include "shapes.h"

// note that it will have to be manually destroyed
TextureRegion* gfx_loadOrGet(PyObject *obj);

Batch* getRendererBatch(PyObject *renderer);
glrenderer_ShapeBatch* getShapeBatch(PyObject *renderer); // borrowed reference
