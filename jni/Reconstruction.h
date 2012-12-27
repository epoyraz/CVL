/*
 * Reconstruction.h
 *
 *  Created on: 22.11.2012
 *      Author: gostefan
 */

#ifndef RECONSTRUCTION_H_
#define RECONSTRUCTION_H_

#include <android/log.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>

#include <QCAR/QCAR.h>
#include <QCAR/CameraDevice.h>
#include <QCAR/Tool.h>

using namespace cv;
using namespace std;

#define LOG(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#ifndef LOG_TAG
#define LOG_TAG "Reconstruction"
#endif

class Reconstruction {
public:
	Reconstruction(int width, int height, int x = 10, int y = 10, int z = 10);
	~Reconstruction();
	unsigned int getTexture();
	void addSilhouette(Mat* silhouette, Mat* mvMatrix);
	float getOverX();
	float getOVerY();
	float getNSlices();

private:
	void calculateSizes();
	void getCameraMatrix();
	/**
	 * Stores the intermediate texture
	 */
	IplImage* voxe;
	/**
	 * Stores the camera matrix
	 */
	Mat* cameraMatrix;
	/**
	 * Stores the boolean value whether a voxel is active or not
	 */
	bool* voxels;
	/**
	 * Stores how many voxels are in x, y and z direction
	 */
	int x, y, z;
	/**
	 * Stores how many texels are in total width and total height
	 */
	int width, height;
	/**
	 * Stores how many slices in x and y direction are stored in the texture
	 */
	int nWidth, nHeight;
	/**
	 * Stores the image width/height
	 */
	int imgWidth, imgHeight;
};


#endif /* RECONSTRUCTION_H_ */
