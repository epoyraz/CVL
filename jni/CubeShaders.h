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
attribute vec4 aVertexColor;\
\
varying vec4 normal; \
varying vec4 frontColor;\
 \
uniform mat4 modelViewProjectionMatrix; \
 \
void main() \
{ \
   gl_Position = modelViewProjectionMatrix * vertexPosition; \
   normal = vertexNormal; \
   frontColor = aVertexColor;\
} \
";


static const char* cubeFragmentShader = " \
 \
precision mediump float; \
 \
varying vec4 frontColor;\
void main() \
{ \
   gl_FragColor = frontColor; \
} \
";

#endif

#endif // _QCAR_CUBE_SHADERS_H_
