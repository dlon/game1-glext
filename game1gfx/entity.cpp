#include "entity/DefaultEntityRenderer.h"
#include <Python.h>

int entity_init(PyObject *glExtModule)
{
	// TODO: create a submodule "entity" to contain the renderers

	if (PyType_Ready(&DefaultEntityRendererType) < 0)
		return -1;

	PyModule_AddObject(glExtModule, "DefaultEntityRenderer", (PyObject *)&DefaultEntityRendererType);

	return 0;
}
