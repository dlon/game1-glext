#pragma once

#include "glutil.hpp"
#include <string>


class GLProgram {
public:
	GLProgram(const char *vertSrc, const char *fragSrc, const char *geomSrc);
	GLProgram(GLProgram &other) = delete;
	~GLProgram();

	bool OK();
	bool compiledOK();
	bool linkedOK();
	const char * getErrorMessage();

	void use();

	GLuint getGLProgram() { return program;  }

private:
	const char *vertSrc;
	const char *fragSrc;
	const char *geomSrc;

	void link();
	void compile();

	enum ErrorState {
		OK_STATE = 0,
		VERT_FAILED,
		FRAG_FAILED,
		GEOM_FAILED,
		LINK_FAILED
	};
	ErrorState errorState = OK_STATE;

	GLuint vertShader;
	GLuint fragShader;
	GLuint geomShader;
	
	GLuint program;
};
