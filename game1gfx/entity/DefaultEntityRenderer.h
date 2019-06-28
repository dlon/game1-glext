#pragma once

#include <Python.h>

extern PyTypeObject DefaultEntityRendererType;

int DefaultEntityRenderer_init();
int DefaultEntityRenderer_cleanup();
