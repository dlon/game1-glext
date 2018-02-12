#include <Windows.h>
#include <gl/GL.h>
#include "batch.hpp"

const char* vertexShader = R"(#version 440

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

const char* fragmentShader = R"(#version 440
    
layout (location = 3) uniform vec4 colorUniform;

in vec2 texCoord0;
in vec4 vertColor;
layout (location = 1) uniform sampler2D u_texture0;

void main(void) {
    gl_FragColor = colorUniform * vertColor * texture2D(u_texture0, texCoord0);
})";


void Batch::setupShaders() {
	//glEnableVertexAttribArray();
}

Batch::Batch() {

}
