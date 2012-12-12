/*==============================================================================
            Copyright (c) 2012 QUALCOMM Austria Research Center GmbH.
            All Rights Reserved.
            Qualcomm Confidential and Proprietary
            
@file 
    Teapot.h

@brief
    Geometry for the teapot used in the samples.

==============================================================================*/

#ifndef _QCAR_TEAPOT_OBJECT_H_
#define _QCAR_TEAPOT_OBJECT_H_


#define NUM_TEAPOT_OBJECT_VERTEX 24
#define NUM_TEAPOT_OBJECT_INDEX  36


static const float teapotVertices[NUM_TEAPOT_OBJECT_VERTEX * 3] = {
		 25, 25, 50,  -25, 25, 50,  -25,-25, 50,   25,-25, 50,   // v0,v20,v2,v3 (front)
		 25, 25, 50,   25,-25, 50,   25,-25,  0,   25, 25,  0,   // v0,v3,v4,v5 (right)
		 25, 25, 50,   25, 25,  0,  -25, 25,  0,  -25, 25, 50,   // v0,v5,v6,v20 (top)
		-25, 25, 50,  -25, 25,  0,  -25,-25,  0,  -25,-25, 50,   // v20,v6,v7,v2 (left)
		-25,-25,  0,   25,-25,  0,   25,-25, 50,  -25,-25, 50,   // v7,v4,v3,v2 (bottom)
		 25,-25,  0,  -25,-25,  0,  -25, 25,  0,   25, 25,  0    // v4,v7,v6,v5
};

static const float teapotTexCoords[NUM_TEAPOT_OBJECT_VERTEX * 2] =
{
};

static const float teapotNormals[NUM_TEAPOT_OBJECT_VERTEX * 3] =
{
		0, 0, 1,   0, 0, 1,   0, 0, 1,   0, 0, 1,   // v0,v1,v2,v3 (front)
		1, 0, 0,   1, 0, 0,   1, 0, 0,   1, 0, 0,   // v0,v3,v4,v5 (right)
		0, 1, 0,   0, 1, 0,   0, 1, 0,   0, 1, 0,   // v0,v5,v6,v1 (top)
	   -1, 0, 0,  -1, 0, 0,  -1, 0, 0,  -1, 0, 0,   // v1,v6,v7,v2 (left)
		0,-1, 0,   0,-1, 0,   0,-1, 0,   0,-1, 0,   // v7,v4,v3,v2 (bottom)
	    0, 0,-1,   0, 0,-1,   0, 0,-1,   0, 0,-1  // v4,v7,v6,v5 (back)
};

static const float color[NUM_TEAPOT_OBJECT_VERTEX*3] =
{
		1, 1, 1,   1, 1, 0,   1, 0, 0,   1, 0, 1,   // v0,v1,v2,v3 (front)
        1, 1, 1,   1, 0, 1,   0, 0, 1,   0, 1, 1,   // v0,v3,v4,v5 (right)
        1, 1, 1,   0, 1, 1,   0, 1, 0,   1, 1, 0,   // v0,v5,v6,v1 (top)
        1, 1, 0,   0, 1, 0,   0, 0, 0,   1, 0, 0,   // v1,v6,v7,v2 (left)
        0, 0, 0,   0, 0, 1,   1, 0, 1,   1, 0, 0,   // v7,v4,v3,v2 (bottom)
        0, 0, 1,   0, 0, 0,   0, 1, 0,   0, 1, 1 	 // v4,v7,v6,v5 (back)
};

static const unsigned short teapotIndices[NUM_TEAPOT_OBJECT_INDEX] =
{
		0, 1, 2,   2, 3, 0,      // front
		4, 5, 6,   6, 7, 4,      // right
		8, 9,10,  10,11, 8,      // top
	   12,13,14,  14,15,12,      // left
	   16,17,18,  18,19,16,      // bottom
	   20,21,22,  22,23,20
};


#endif // _QCAR_TEAPOT_OBJECT_H_
