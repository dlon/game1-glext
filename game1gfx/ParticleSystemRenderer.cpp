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

	const Color &initialColor = ps->getDefinition().initialColor.min;
	ShapeBatch_setColor(shapes, initialColor.r, initialColor.g, initialColor.b, initialColor.a);

	if (!particles.hasColor()) {
		if (initialColor.a == 1.f) {
			ShapeBatch_disableBlending(shapes);
		}
		else {
			ShapeBatch_enableBlending(shapes);
		}
	}

	for (int i = 0; i < components; i += cPerParticle) {
		if (particles.get(i + 0) < .0f) // longevity
			continue;

		float x = particles.get(i + 1);
		float y = particles.get(i + 2);

		if (particles.hasColor()) {
			int relativeComponent = 3;
			if (particles.hasVelocity()) {
				relativeComponent += ParticleArray::VELOCITY_NUM_COMPONENTS;
			}

			if (particles.hasAlpha()) {
				ShapeBatch_setColor(
					shapes,
					particles.get(i + relativeComponent + 0),
					particles.get(i + relativeComponent + 1),
					particles.get(i + relativeComponent + 2),
					particles.get(i + relativeComponent + 3)
				);
			}
			else {
				ShapeBatch_setColor(
					shapes,
					particles.get(i + relativeComponent + 0),
					particles.get(i + relativeComponent + 1),
					particles.get(i + relativeComponent + 2),
					initialColor.a
				);
			}
		}

		ShapeBatch_drawLines(
			shapes,
			2,
			x + point0x, y + point0y,
			x + point1x, y + point1y
		);
	}

	ShapeBatch_end(shapes);
	ShapeBatch_enableBlending(shapes);
	glEnable(GL_BLEND);
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
