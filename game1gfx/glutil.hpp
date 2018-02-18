#pragma once

#include <Windows.h>
#include <gl/GL.h>
#include "glext.h"

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

extern GLuint(*glCreateProgram)();
extern void(*glUseProgram)(GLuint program);
extern void(*glLinkProgram)(GLuint program);
extern void(*glDeleteProgram)(GLuint program);
extern void(*glGetProgramiv)(GLuint program,
	GLenum pname,
	GLint *params);

extern GLuint(*glCreateShader)(GLenum shaderType);
extern void(*glShaderSource)(GLuint shader,
	GLsizei count,
	const char **string,
	const GLint *length);
extern void(*glCompileShader)(GLuint shader);
extern void(*glAttachShader)(GLuint program, GLuint shader);
extern void(*glDeleteShader)(GLuint shader);
extern void(*glGetShaderiv)(GLuint shader,
	GLenum pname,
	GLint *params);

extern void(*glActiveTexture)(GLenum texture);

extern void(*glDeleteBuffers)(GLsizei n, const GLuint * buffers);
extern void(*glGenBuffers)(GLsizei n, GLuint * buffers);
extern void(*glBindBuffer)(GLenum target, GLuint buffer);
extern void(*glBufferData)(GLenum target,
	ptrdiff_t size,
	const GLvoid * data,
	GLenum usage);
extern void(*glBufferSubData)(GLenum target,
	GLintptr offset,
	GLsizeiptr size,
	const GLvoid * data);
extern void(*glGenVertexArrays)(GLsizei n, GLuint * buffers);
extern void(*glBindVertexArray)(GLuint buffer);

extern void(*glUniformMatrix3fv)(GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value);
extern void(*glUniform1f)(GLint location, GLfloat v0);
extern void(*glUniform1i)(GLint location,
	GLint v0);
extern void(*glUniform4f)(GLint location,
	GLfloat v0,
	GLfloat v1,
	GLfloat v2,
	GLfloat v3);
