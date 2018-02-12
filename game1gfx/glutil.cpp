#include <Windows.h>
#include <gl/GL.h>
#include "glutil.hpp"
#include <stdio.h>

void (*glEnableVertexAttribArray)(GLuint index) = NULL;
void(*glVertexAttribPointer)(GLuint index,
	GLint size,
	GLenum type,
	GLboolean normalized,
	GLsizei stride,
	const GLvoid * pointer) = NULL;
GLint (*glGetAttribLocation)(GLuint program, const char *name) = NULL;
GLint(*glGetUniformLocation)(GLuint program, const char *name) = NULL;
void(*glUseProgram)(GLuint program) = NULL;
void(*glActiveTexture)(GLenum texture) = NULL;
void(*glDeleteBuffers)(GLsizei n, const GLuint * buffers) = NULL;
void(*glGenBuffers)(GLsizei n, GLuint * buffers) = NULL;

void loadGLFunctions() {
	glEnableVertexAttribArray = (void(*)(GLuint))wglGetProcAddress("glEnableVertexAttribArray");
	glVertexAttribPointer = (void(*)(GLuint index,
		GLint size,
		GLenum type,
		GLboolean normalized,
		GLsizei stride,
		const GLvoid * pointer))wglGetProcAddress("glVertexAttribPointer");
	glGetAttribLocation = (GLint(*)(GLuint, const char*))wglGetProcAddress("glGetAttribLocation");
	glGetUniformLocation = (GLint(*)(GLuint, const char*))wglGetProcAddress("glGetUniformLocation");
	glUseProgram = (void(*)(GLuint))wglGetProcAddress("glUseProgram");
	glActiveTexture = (void(*)(GLenum))wglGetProcAddress("glActiveTexture");
	glDeleteBuffers = (void(*)(GLsizei, const GLuint*))wglGetProcAddress("glDeleteBuffers");
	glGenBuffers = (void(*)(GLsizei, GLuint*))wglGetProcAddress("glGenBuffers");
	
	puts("Loaded gl functions");
}
