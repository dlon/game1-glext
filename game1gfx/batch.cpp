#include <Windows.h>
#include <gl/GL.h>
#include "batch.hpp"
#include "glutil.hpp"
#include <vector>

const char vertexShaderSource[] = R"(#version 440

layout(location = 0) uniform mat3 vpMatrix;
layout(location = 2) uniform mat3 mMatrix;

layout(location = 0) in vec2 vPosition;
layout(location = 1) in vec2 vTexCoord0;
layout(location = 2) in vec4 vVertColor;

out vec2 texCoord0;
out vec4 vertColor;

void main(void) {
	gl_Position = vec4(vpMatrix * mMatrix * vec3(vPosition, 1.0), 1.0);
	texCoord0 = vTexCoord0;
	vertColor = vVertColor;
})";

const char fragmentShaderSource[] = R"(#version 440
    
layout (location = 3) uniform vec4 colorUniform;

in vec2 texCoord0;
in vec4 vertColor;
layout (location = 1) uniform sampler2D u_texture0;

void main(void) {
    gl_FragColor = colorUniform * vertColor * texture2D(u_texture0, texCoord0);
})";


void Batch::setupShaders() {
	const char* vshaders[] = { vertexShaderSource };
	const char* fshaders[] = { fragmentShaderSource };

	program = glCreateProgram();

	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(
		vertexShader,
		1,
		vshaders,
		NULL
	);
	glCompileShader(vertexShader);

	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(
		fragmentShader,
		1,
		fshaders,
		NULL
	);
	glCompileShader(fragmentShader);

	// TODO: error checking

	glEnableVertexAttribArray(positionAttribute);
	glEnableVertexAttribArray(texCoordAttribute);
	glEnableVertexAttribArray(colorAttribute);
}

void Batch::createBuffers() {
	std::vector<size_t> indices(4 * maxBatchSize);

	for (int i = 0; i < maxBatchSize; i++) {
		// TODO: pre-generate this
		indices.push_back(4 * i + 0);
		indices.push_back(4 * i + 2);
		indices.push_back(4 * i + 1);
		indices.push_back(4 * i + 2);
		indices.push_back(4 * i + 1);
		indices.push_back(4 * i + 3);
	}

	glGenBuffers(1, &indexVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVbo);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		sizeof(size_t) * indices.size(),
		&indices[0],
		GL_STATIC_DRAW
	);
}

Batch::Batch(size_t maxBatchSize) {
	setupShaders();
	
	this->maxBatchSize = maxBatchSize;
	objectIndex = 0;
	currentTexture = NULL;

	createBuffers();
}

Batch::~Batch() {
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteProgram(program);
}
