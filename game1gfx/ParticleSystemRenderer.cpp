#include "ParticleSystemRenderer.h"


static void renderLineParticles(
	glrenderer_ShapeBatch *shapes,
	Batch *batch,
	ParticleSystem * ps
) {
	ParticleArray& particles = ps->getParticles();
	int cPerParticle = particles.getComponentsPerParticle();
	int components = particles.getNumComponents();

	const ParticleVertex* points = ps->getDefinition().appearance.points;
	float point0x = points[0].x;
	float point0y = points[0].y;
	float point1x = points[1].x;
	float point1y = points[1].y;

	ShapeBatch_begin(shapes);

	ShapeBatch_ignoreCamera(shapes);
	ShapeBatch_setColor(shapes, 1.f, 1.f, 1.f, 1);

	for (int i = 0; i < components; i += cPerParticle) {
		if (particles.get(i + 0) < .0f) // longevity
			continue;

		float x = particles.get(i + 1);
		float y = particles.get(i + 2);

		// TODO: handle extra properties

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
			//x + 5.f * 0.342f,
			//y - 5.f * 0.93969f,
			x + point0x, y + point0y,
			x + point1x, y + point1y
		);
	}

	ShapeBatch_end(shapes);
}


void ParticleSystemRenderer::render(
	glrenderer_ShapeBatch *shapes,
	Batch *batch,
	ParticleSystem * ps
) {
	switch (ps->getDefinition().appearance.type) {
	case ParticleAppearance::Line:
		renderLineParticles(shapes, batch, ps);
		break;
	}

	// TODO: circle
	// TODO: sprite
	// TODO: circles, sprite: use points[0] as offset
}

void ParticleSystemRenderer::render(glrenderer_ShapeBatch * shapes, Batch * batch, const ParticleSystemCollection& collection)
{
	for (auto it = collection.begin(); it != collection.end(); ++it) {
		render(shapes, batch, (*it));
	}
}
