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
	char getAt(int x, int y, int z);
	void setAt(int x, int y, int z, char value);
	IplImage* voxe;
	Mat* cameraMatrix;
	bool* voxels;
	int x, y, z;
	int width, height;
	int nWidth, nHeight;
	int imgWidth, imgHeight;
};


#endif /* RECONSTRUCTION_H_ */
