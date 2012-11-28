#ifndef _QCAR_SIMPLEFRAGMENT_SHADERS_H_
#define _QCAR_SIMPLEFRAGMENT_SHADERS_H_

#ifndef USE_OPENGL_ES_1_1

static const char* simpleFragmentShader = " \
varying vec4 backColor;\
\
void main(void)\
{\
	gl_FragColor = backColor;\
}\
";
#endif

#endif
