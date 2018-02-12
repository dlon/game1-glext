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
void(*glLinkProgram)(GLuint program) = NULL;
void(*glActiveTexture)(GLenum texture) = NULL;
void(*glDeleteBuffers)(GLsizei n, const GLuint * buffers) = NULL;
void(*glGenBuffers)(GLsizei n, GLuint * buffers) = NULL;
void(*glBindBuffer)(GLenum target, GLuint buffer) = NULL;
void(*glBufferData)(GLenum target,
	ptrdiff_t size,
	const GLvoid * data,
	GLenum usage) = NULL;
void (*glUniformMatrix3fv)(GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value) = NULL;
void (*glUniform1f)(GLint location, GLfloat v0) = NULL;

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
	glLinkProgram = (void(*)(GLuint))wglGetProcAddress("glLinkProgram");
	glActiveTexture = (void(*)(GLenum))wglGetProcAddress("glActiveTexture");
	glDeleteBuffers = (void(*)(GLsizei, const GLuint*))wglGetProcAddress("glDeleteBuffers");
	glGenBuffers = (void(*)(GLsizei, GLuint*))wglGetProcAddress("glGenBuffers");
	glBindBuffer = (void(*)(GLenum target, GLuint buffer))wglGetProcAddress("glBindBuffer");
	glBufferData = (void(*)(GLenum target,
		ptrdiff_t size,
		const GLvoid * data,
		GLenum usage))wglGetProcAddress("glBufferData");
	glUniformMatrix3fv = (void(*)(GLint location,
		GLsizei count,
		GLboolean transpose,
		const GLfloat *value))wglGetProcAddress("glUniformMatrix3fv");
	glUniform1f = (void(*)(GLint location, GLfloat v0))wglGetProcAddress("glUniform1f");
	
	puts("Loaded gl functions");
}
