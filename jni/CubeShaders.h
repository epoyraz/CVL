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
   pos = modelViewProjectionMatrix * vertexPosition; \
   gl_Position = pos;\
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
\
uniform vec3 eyePos;\
\
const float steps = 50.0;\
const float numberOfSlices = 10.0;\
const float slicesOverX = 10.0;\
const float slicesOverY = 10.0;\
\
\
float getVolumeValue(vec3 volpos)\
{\
	float s1,s2;\
	float dx1,dy1;\
	float dx2,dy2;\
\
	vec2 texpos1,texpos2;\
\
	s1 = floor(volpos.z*numberOfSlices);\
	s2 = s1+1.0;\
\
	dx1 = fract(s1/slicesOverX);\
	dy1 = floor(s1/slicesOverY)/slicesOverY;\
\
	dx2 = fract(s2/slicesOverX);\
	dy2 = floor(s2/slicesOverY)/slicesOverY;\
\
	texpos1.x = dx1+(volpos.x/slicesOverX);\
	texpos1.y = dy1+(volpos.y/slicesOverY);\
\
	texpos2.x = dx2+(volpos.x/slicesOverX);\
	texpos2.y = dy2+(volpos.y/slicesOverY);\
\
	return mix( texture2D(uVolData,texpos1).x, texture2D(uVolData,texpos2).x, (volpos.z*numberOfSlices)-s1);\
}\
\
void main() \
{ \
	vec3 dir = frontColor.rgb - eyePos.rgb;\
    dir *= 1.74 / length(dir);\
\
	float cont = 0.0;\
	vec3 Step = dir/steps;\
\
	float opacityFactor = 8.0;\
	float lightFactor = 1.3;\
\
	vec4 sample = vec4(0.0, 0.0, 0.0, opacityFactor * (1.0/steps));\
	vec4 value = vec4(0, 0, 0, 0);\
\
	vec4 accum = vec4(0, 0, 0, 0);\
	vec4 vpos = frontColor;\
	for(float i=0.0;i<steps;i+=1.0)\
	{\
		vec2 tf_pos;\
		tf_pos.x = getVolumeValue(vpos.xyz);\
		value = vec4(tf_pos.x);\
\
		sample.rgb = value.rgb * sample.a * lightFactor;\
		accum.rgb += (1.0 - accum.a) * sample.rgb;\
		accum.a += sample.a;\
		vpos.xyz = vpos.xyz + Step.xyz;\
		if(vpos.x > 1.0 || vpos.y > 1.0 || vpos.z > 1.0 || accum.a>=1.0)\
		    break;\
	}\
    accum.a *= 0.8;\
	gl_FragColor = accum;\
} \
";

#endif

#endif // _QCAR_CUBE_SHADERS_H_
