#ifndef _QCAR_SIMPLEVERTEX_SHADERS_H_
#define _QCAR_SIMPLEVERTEX_SHADERS_H_

#ifndef USE_OPENGL_ES_1_1

static const char* simpleVertexShader = " \
attribute vec3 aVertexPosition;\
attribute vec4 aVertexColor;\
\
uniform mat4 uMVMatrix;\
uniform mat4 uPMatrix;\
\
varying vec4 backColor;\
\
void main(void)\
{\
	vec4 pos = uPMatrix * uMVMatrix * vec4(aVertexPosition, 1.0);\
	gl_Position = pos;\
	backColor = aVertexColor;\
}\
";
#endif

#endif
