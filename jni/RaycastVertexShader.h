#ifndef _QCAR_RAYCASTVERTEX_SHADERS_H_
#define _QCAR_RAYCASTVERTEX_SHADERS_H_

#ifndef USE_OPENGL_ES_1_1

static const char* raycastVertexShader = " \
attribute vec3 aVertexPosition;\
attribute vec4 aVertexColor;\
\
uniform mat4 uMVMatrix;\
uniform mat4 uPMatrix;\
\
varying vec4 frontColor;\
varying vec4 pos;\
\
void main(void)\
{\
	pos = uPMatrix * uMVMatrix * vec4(aVertexPosition, 1.0);\
	gl_Position = pos;\
	frontColor = aVertexColor;\
}\
";
#endif

#endif
