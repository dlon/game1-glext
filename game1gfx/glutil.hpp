#pragma once

#include <Windows.h>
#include <gl/GL.h>

extern void loadGLFunctions();

extern void(*glEnableVertexAttribArray)(GLuint index);
extern GLint(*glGetAttribLocation)(GLuint program, const char *name);
extern GLint(*glGetUniformLocation)(GLuint program, const char *name);
extern void(*glUseProgram)(GLuint program);
