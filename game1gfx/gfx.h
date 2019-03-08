#pragma once

#include "TextureRegion.h"

TextureRegion* gfx_loadOrGet(PyObject *obj);
Batch* getRendererBatch(PyObject *renderer);
