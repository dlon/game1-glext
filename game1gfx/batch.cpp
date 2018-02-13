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

	// TODO: error checking & exceptions

	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	glEnableVertexAttribArray(positionAttribute);
	glEnableVertexAttribArray(texCoordAttribute);
	glEnableVertexAttribArray(colorAttribute);

	GLfloat projectionMatrix[3][3] = {
		{ 2.0f/320.0f, 0, 0 },
		{ 0, -2.0f / 240.0f, 0 },
		{ 0, 0, 1 }
	};
	GLfloat viewMatrix[3][3] = {
		{ 1, 0, 0 },
		{ 0, 1, 0 },
		{ -160, -120, 1 }
	};

	GLfloat matrix[3][3] = { 0 };
	for (int i = 0; i<3; i++) {
		for (int j = 0; j<3; j++) {
			for (int k = 0; k<3; k++) {
				matrix[i][j] += viewMatrix[i][k] * projectionMatrix[k][j];
			}
		}
	}
	// kan vara fel
	
	glUseProgram(program);
	glUniformMatrix3fv(
		vpMatrixUniform,
		1,
		GL_FALSE,
		(GLfloat*)matrix
	);
	glUniform1i(textureUniform, 0);
}

void Batch::createBuffers() {
	GLuint* indices = new GLuint[6 * maxBatchSize];
	for (int i = 0; i < maxBatchSize; i++) {
		// TODO: pre-generate this
		indices[6 * i + 0] = 6 * i + 0;
		indices[6 * i + 1] = 6 * i + 2;
		indices[6 * i + 2] = 6 * i + 1;
		indices[6 * i + 3] = 6 * i + 2;
		indices[6 * i + 4] = 6 * i + 1;
		indices[6 * i + 0] = 6 * i + 3;
	}
	printf("%d\n", sizeof(indices));

	glGenBuffers(1, &indexVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVbo);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		sizeof(GLuint) * (6 * maxBatchSize),
		indices,
		GL_STATIC_DRAW
	);

	delete[] indices;

	glGenBuffers(1, &vertexVbo);
	glBindBuffer(GL_ARRAY_BUFFER, vertexVbo);
	vertexAttribData.resize(4 * 8 * maxBatchSize);
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(attributeType) * vertexAttribData.size(),
		vertexAttribData.data(),
		GL_DYNAMIC_DRAW
	);
}

Batch::Batch(size_t maxBatchSize) {
	this->maxBatchSize = maxBatchSize;
	objectIndex = 0;
	currentTexture = NULL;

	setupShaders();
	createBuffers();

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
}

Batch::~Batch() {
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteProgram(program);
}
