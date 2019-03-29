#pragma once

#include "TextureRegion.h"

// note that it will have to be manually destroyed
TextureRegion* gfx_loadOrGet(PyObject *obj);

Batch* getRendererBatch(PyObject *renderer);
