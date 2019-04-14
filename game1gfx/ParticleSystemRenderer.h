#pragma once

#include "glutil.hpp"
#include <Windows.h>
#include <gl/GL.h>
#include "ParticleSystem.h"
#include "ParticleSystemCollection.h"
#include "shapes.h"
#include "Batch.hpp"

class ParticleSystemRenderer
{
public:
	void render(
		glrenderer_ShapeBatch *shapes,
		Batch *batch,
		ParticleSystem * ps
	);
	void render(
		glrenderer_ShapeBatch * shapes,
		Batch * batch,
		const ParticleSystemCollection& collection
	);
};
