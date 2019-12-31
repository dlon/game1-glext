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
	glDeleteShader(vertShader);
	if (errorState ^ VERT_FAILED) {
		return;
	}
	glDeleteShader(fragShader);
	if (errorState ^ FRAG_FAILED) {
		return;
	}
	if (geomSrc) {
		glDeleteShader(geomShader);
		if (errorState ^ GEOM_FAILED) {
			return;
		}
	}
	glDeleteProgram(program);
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
	if (program == 0) {
		errorState = LINK_FAILED;
		return;
	}
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

/*static void concatProgramInfoLog(GLuint program, char *buf, size_t maxsize)
{
	size_t curLen = strlen(buf);
	if (curLen + 1 >= maxsize)
		return;
	glGetProgramInfoLog(program, maxsize - curLen, 0, buf);
}*/

static void concatShaderInfoLog(GLuint shader, char *buf, size_t maxsize)
{
	size_t curLen = strlen(buf);
	if (curLen + 1 >= maxsize)
		return;
	glGetShaderInfoLog(shader, maxsize - curLen, nullptr, buf + curLen);
}

const char *GLProgram::getErrorMessage()
{
	static char infoBuffer[1000];

	switch (errorState) {
	case OK_STATE:
		return nullptr;

	case VERT_FAILED:
		strcpy(infoBuffer, "Vertex shader compilation error: ");
		concatShaderInfoLog(vertShader, infoBuffer, sizeof(infoBuffer));
		return infoBuffer;
	case FRAG_FAILED:
		strcpy(infoBuffer, "Fragment shader compilation error: ");
		concatShaderInfoLog(fragShader, infoBuffer, sizeof(infoBuffer));
		return infoBuffer;
	case GEOM_FAILED:
		strcpy(infoBuffer, "Geometry shader compilation error: ");
		concatShaderInfoLog(geomShader, infoBuffer, sizeof(infoBuffer));
		return infoBuffer;
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
