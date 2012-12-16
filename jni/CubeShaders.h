/*==============================================================================
            Copyright (c) 2012 QUALCOMM Austria Research Center GmbH.
            All Rights Reserved.
            Qualcomm Confidential and Proprietary
            
@file 
    CubeShaders.h

@brief
    Defines OpenGL shaders as char* strings.

==============================================================================*/

#ifndef _QCAR_CUBE_SHADERS_H_
#define _QCAR_CUBE_SHADERS_H_

#ifndef USE_OPENGL_ES_1_1

static const char* cubeMeshVertexShader = " \
  \
attribute vec4 vertexPosition; \
attribute vec4 vertexNormal; \
attribute vec4 vertexTexCoord; \
attribute vec4 aVertexColor;\
\
varying vec4 frontColor;\
varying vec4 normal; \
varying vec4 pos;\
\
uniform mat4 modelViewProjectionMatrix; \
 \
void main() \
{ \
   pos = vertexPosition; \
   gl_Position = modelViewProjectionMatrix * vertexPosition;\
   normal = vertexNormal; \
   frontColor = aVertexColor;\
} \
";


//return mix( texture2D(texSampler2D,texpos1).x, texture2D(texSampler2D,texpos2).x, (volpos.z*numberOfSlices)-s1);\

static const char* cubeFragmentShader = " \
\
precision mediump float; \
\
varying vec4 normal; \
varying vec4 frontColor;\
varying vec4 pos;\
\
uniform sampler2D uVolData; \
uniform vec3 eyePos;\
uniform vec2 slicesOver;\
uniform float numberOfSlices;\
\
const float steps = 50.0;\
\
\
float getVolumeValue(vec3 volpos)\
{\
	float s1,s2;\
	float dx1,dy1;\
	float dx2,dy2;\
\
	vec2 texpos1;\
\
	s1 = floor(volpos.z*numberOfSlices);\
\
	dx1 = fract(s1/slicesOver.x);\
	dy1 = floor(s1/slicesOver.x)/slicesOver.y;\
\
	texpos1.x = dx1+(volpos.x/slicesOver.x);\
	texpos1.y = dy1+(volpos.y/slicesOver.y);\
\
	/*return mix( texture2D(uVolData,texpos1).x, texture2D(uVolData,texpos2).x, (volpos.z*numberOfSlices)-s1);*/\
    return texture2D(uVolData, texpos1).x;\
}\
\
void main() \
{ \
	vec3 dir = pos.rgb - eyePos.rgb;\
    dir *= 1.74 / length(dir);\
\
	vec3 Step = dir/steps;\
\
	float opacityFactor = 8.0;\
	float lightFactor = 1.3;\
\
	vec4 sample = vec4(0.0, 0.0, 0.0, opacityFactor / steps);\
	vec4 value = vec4(0, 0, 0, 0);\
\
	vec4 accum = vec4(0, 0, 0, 0);\
	vec4 vpos = frontColor;\
    vpos.x *= 0.99;\
	for(float i=0.0;i<steps;i+=1.0)\
	{\
		float tex = getVolumeValue(vpos.zyx);\
        if (tex > 0.9) {\
            accum = vec4(1,1,1,1);\
		    break;\
        }\
\
		vpos.xyz += Step.zyx;\
		if(vpos.x > 1.0 || vpos.y > 1.0 || vpos.z > 1.0 || accum.a >=1.0 || \
		     vpos.x < 0.0 || vpos.y < 0.0 || vpos.z < 0.0)\
		    break;\
	}\
\
    accum.a *= 0.8;\
	gl_FragColor = accum;\
} \
";

#endif

#endif // _QCAR_CUBE_SHADERS_H_
