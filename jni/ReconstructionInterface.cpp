/*
 * ReconstructionInterface.cpp
 *
 *  Created on: 22.11.2012
 *      Author: gostefan
 */

#ifndef RECONSTRUCTION_INT_H_
#define RECONSTRUCTION_INT_H_

#include <jni.h>
#include <android/log.h>
#include "S3D.h"

Reconstruction* reconstructionHandler = NULL;

extern "C"
{
	/**
	 * Instantiates the reconstruction
	 */
	JNIEXPORT void JNICALL Java_edu_ethz_s3d_Reconstruction_generateReconstruction(JNIEnv* env, jobject, jint x, jint y, jint z)
	{
		if (reconstructionHandler != NULL)
			delete reconstructionHandler;

		int width = grabCutObject->getWidth();
		int height = grabCutObject->getHeight();

		reconstructionHandler = new Reconstruction(width, height, x, y, z);
	}

	/**
	 * Adds the latest silhouette to the reconstruction
	 */
	JNIEXPORT void JNICALL Java_edu_ethz_s3d_Reconstruction_refineWithLatestCut(JNIEnv* env, jobject)
	{
		Mat* silhouette = sStorage->getLastSilhouette();
		Mat* mvMat = sStorage->getLastMVMatrice();
		reconstructionHandler->addSilhouette(silhouette, mvMat);
	}

}

#endif
