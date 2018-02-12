#pragma once

#include <Windows.h>
#include <gl/GL.h>

extern void loadGLFunctions();

extern void(*glEnableVertexAttribArray)(GLuint index);
extern void(*glVertexAttribPointer)(GLuint index,
	GLint size,
	GLenum type,
	GLboolean normalized,
	GLsizei stride,
	const GLvoid * pointer);
extern GLint(*glGetAttribLocation)(GLuint program, const char *name);
extern GLint(*glGetUniformLocation)(GLuint program, const char *name);
extern void(*glUseProgram)(GLuint program);
extern void(*glLinkProgram)(GLuint program);
extern void(*glActiveTexture)(GLenum texture);
extern void(*glDeleteBuffers)(GLsizei n, const GLuint * buffers);
extern void(*glGenBuffers)(GLsizei n, GLuint * buffers);
extern void(*glBindBuffer)(GLenum target, GLuint buffer);
extern void(*glBufferData)(GLenum target,
	ptrdiff_t size,
	const GLvoid * data,
	GLenum usage);
extern void(*glUniformMatrix3fv)(GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value);
extern void(*glUniform1f)(GLint location, GLfloat v0);
