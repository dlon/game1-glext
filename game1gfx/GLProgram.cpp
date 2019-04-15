#include "GLProgram.h"
#include "glutil.hpp"


GLProgram::GLProgram(const char * vertSrc, const char * fragSrc, const char * geomSrc) :
	vertSrc(vertSrc), fragSrc(fragSrc), geomSrc(geomSrc)
{
	compile();
	if (compiledOK())
		link();
}

GLProgram::~GLProgram()
{
	if (errorState ^ VERT_FAILED) {
		glDeleteShader(vertShader);
	}
	if (errorState ^ FRAG_FAILED) {
		glDeleteShader(fragShader);
	}
	if (geomSrc && errorState ^ GEOM_FAILED) {
		glDeleteShader(geomShader);
	}
	if (errorState ^ LINK_FAILED) {
		glDeleteProgram(program);
	}
}

static bool compileShader(GLenum shaderType, const char *src, GLuint *ret)
{
	const char* sources[] = { src, nullptr };
	*ret = glCreateShader(shaderType);
	glShaderSource(*ret, 1, sources, NULL);
	glCompileShader(*ret);

	GLint isCompiled = 0;
	glGetShaderiv(*ret, GL_COMPILE_STATUS, &isCompiled);
	if (!isCompiled) {
		return false;
	}
	return true;
}

void GLProgram::compile()
{
	if (!compileShader(GL_VERTEX_SHADER, vertSrc, &vertShader)) {
		errorState = VERT_FAILED;
		return;
	}
	if (!compileShader(GL_FRAGMENT_SHADER, fragSrc, &fragShader)) {
		errorState = FRAG_FAILED;
		return;
	}
	if (geomSrc && !compileShader(GL_GEOMETRY_SHADER, geomSrc, &geomShader)) {
		errorState = GEOM_FAILED;
		return;
	}
}

void GLProgram::link()
{
	//if (!compiledOK())
	//	oops...

	program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	if (geomSrc)
		glAttachShader(program, geomShader);
	glLinkProgram(program);

	GLint isLinked;
	glGetProgramiv(
		program,
		GL_LINK_STATUS,
		&isLinked
	);

	if (!isLinked)
		errorState = LINK_FAILED;
}

bool GLProgram::OK()
{
	return errorState == OK_STATE;
}

bool GLProgram::compiledOK()
{
	return !(errorState & (VERT_FAILED | FRAG_FAILED | GEOM_FAILED));
}

bool GLProgram::linkedOK()
{
	return !(errorState & LINK_FAILED);
}

const char *GLProgram::getErrorMessage()
{
	switch (errorState) {
	case OK_STATE:
		return nullptr;

	case VERT_FAILED:
		return getShaderInfoLog(vertShader);
	case FRAG_FAILED:
		return getShaderInfoLog(fragShader);
	case GEOM_FAILED:
		return getShaderInfoLog(geomShader);
	case LINK_FAILED:
		return getProgramInfoLog(program);
	}

	// NOT SUPPOSED TO HAPPEN
	return nullptr;
}

void GLProgram::use()
{
	// TODO: error handling?
	glUseProgram(program);
}
