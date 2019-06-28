#pragma once

#include <Python.h>

int entity_init(PyObject *glExtModule);
int entity_cleanup();

PyObject* getEntityRegion(PyObject *entity);
int getEntityPos(PyObject *entity, float *x, float *y);
void getEntityImageOffset(PyObject *entity, float *x, float *y);
void getEntityParallax(PyObject *entity, float *x, float *y);
