#include "ParticleSystemRenderer.h"


void ParticleSystemRenderer::render(
	glrenderer_ShapeBatch *shapes,
	Batch *batch,
	ParticleSystem * ps
) {
	// TODO: test. render rain

	ParticleArray& particles = ps->getParticles();
	int cPerParticle = particles.getComponentsPerParticle();
	int components = particles.getNumComponents();

	ShapeBatch_begin(shapes);

	ShapeBatch_ignoreCamera(shapes);
	ShapeBatch_setColor(shapes, 1.f, 1.f, 1.f, 1);

	for (int i = 0; i < components; i += cPerParticle) {
		if (particles.get(i + 0) < .0f) // longevity
			continue;

		float x = particles.get(i + 1);
		float y = particles.get(i + 2);

		/*ShapeBatch_setColor(
			shapes,
			particles.get(i + 5),
			particles.get(i + 6),
			particles.get(i + 7),
			particles.get(i + 8)
		);
		ShapeBatch_drawCircle(shapes, x, y, 5.f * particles.get(i + 9), 3);*/

		ShapeBatch_drawLines(
			shapes,
			2,
			x + 5.f * 0.342f,
			y - 5.f * 0.93969f,
			x, y
		);
	}

	ShapeBatch_end(shapes);
}

void ParticleSystemRenderer::render(glrenderer_ShapeBatch * shapes, Batch * batch, const ParticleSystemCollection& collection)
{
	for (auto it = collection.begin(); it != collection.end(); ++it) {
		render(shapes, batch, (*it));
	}
}
