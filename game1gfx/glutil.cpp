#include <Windows.h>
#include <gl/GL.h>
#include "glutil.hpp"
#include <stdio.h>

void (*glEnableVertexAttribArray)(GLuint index) = NULL;
GLint (*glGetAttribLocation)(GLuint program, const char *name) = NULL;
GLint(*glGetUniformLocation)(GLuint program, const char *name) = NULL;
void(*glUseProgram)(GLuint program) = NULL;

void loadGLFunctions() {
	glEnableVertexAttribArray = (void(*)(GLuint))wglGetProcAddress("glEnableVertexAttribArray");
	glGetAttribLocation = (GLint(*)(GLuint, const char*))wglGetProcAddress("glGetAttribLocation");
	glGetUniformLocation = (GLint(*)(GLuint, const char*))wglGetProcAddress("glGetUniformLocation");
	glUseProgram = (void(*)(GLuint))wglGetProcAddress("glUseProgram");
	
	puts("Loaded gl functions");
}
