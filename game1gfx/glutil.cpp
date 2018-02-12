#include <Windows.h>
#include <gl/GL.h>
#include "glutil.hpp"
#include <stdio.h>

void (*glEnableVertexAttribArray)(GLuint index) = NULL;
GLint (*glGetAttribLocation)(GLuint program, const char *name) = NULL;

void loadGLFunctions() {
	glEnableVertexAttribArray = (void(*)(GLuint))wglGetProcAddress("glEnableVertexAttribArray");
	glGetAttribLocation = (GLint(*)(GLuint, const char*))wglGetProcAddress("glGetAttribLocation");
	
	puts("Loaded gl functions");
}
