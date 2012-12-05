/*
 * Reconstruction.cpp
 *
 *  Created on: 22.11.2012
 *      Author: gostefan
 */

#include "Reconstruction.h"


Reconstruction::Reconstruction(int x_in, int y_in, int z_in) :
	x(x_in), y(y_in), z(z_in) {
	LOG("Allocating Voxels. x: %d , y: %d , z: %d ", x, y, z);
	//TODO: Use the following lines again:
	//voxels = new bool[x * y * z];
	//memset(voxels, (unsigned char)255, x*y*z*sizeof(bool));
	voxels = new bool[x * y * 100];
	//memset(voxels, (unsigned char)255, x*y*z*sizeof(bool));
	memset(voxels, (unsigned char)255, x*y*100*sizeof(bool));
	LOG("All voxels set true.");

	calculateSizes();
	LOG("Sizes calculated.");

	voxe = cvCreateImage(cvSize(width, height) , IPL_DEPTH_8U, 4);
	memset(voxe->imageData, (uchar) 255, width*height*4*sizeof(uchar));
	LOG("Image all voxels set true.");

	getCameraMatrix();
	LOG("Set Camera Matrix");
}

void Reconstruction::getCameraMatrix() {
	LOG("Getting Camera Matrix");
	const QCAR::CameraCalibration& cameraCalibration = QCAR::CameraDevice::getInstance().getCameraCalibration();
    QCAR::Matrix44F camMat = QCAR::Tool::getProjectionGL(cameraCalibration, 2.0f, 2000.0f);

	// We need to allocate the data on the heap
	float* data = new float[16];
	for (int i = 0; i < 4*4; i++) {
		// Transpose matrix
		data[i] = camMat.data[(i%4)*4+(i/4)];
	}
	LOG ("Allocated Memory for camera-Matrix and copied data.");

	// Create matrix
	cameraMatrix = new Mat(4, 4, DataType<float>::type, data);
	LOG("Camera Matrix");
	LOG(" %f %f %f %f", cameraMatrix->at<float>(0,0), cameraMatrix->at<float>(0,1), cameraMatrix->at<float>(0,2), cameraMatrix->at<float>(0,3));
	LOG(" %f %f %f %f", cameraMatrix->at<float>(1,0), cameraMatrix->at<float>(1,1), cameraMatrix->at<float>(1,2), cameraMatrix->at<float>(1,3));
	LOG(" %f %f %f %f", cameraMatrix->at<float>(2,0), cameraMatrix->at<float>(2,1), cameraMatrix->at<float>(2,2), cameraMatrix->at<float>(2,3));
	LOG(" %f %f %f %f", cameraMatrix->at<float>(3,0), cameraMatrix->at<float>(3,1), cameraMatrix->at<float>(3,2), cameraMatrix->at<float>(3,3));
}

void Reconstruction::calculateSizes() {
	// Calculate how many pixels we need
	int prod = x * y * z;
	// Calculate the square root to get the best distribution on both axis
	//TODO: Use the following line again:
	//width = ceil(sqrt((float)prod));
	width = 100;
	// Get the number of images in first direction (makes the first dimension larger)
	nWidth = ceil((float)width/(float)x);
	// Calculate the number of images in second direction
	nHeight = ceil((float)z/(float)nWidth);
	// Calculate height of texture
	height = nHeight * y;
}

Reconstruction::~Reconstruction() {
	delete voxels;
	cvReleaseImage(&voxe);
	delete cameraMatrix->data;
	delete cameraMatrix;
}

unsigned int Reconstruction::getTexture() {
	LOG("Starting texture loading");
	unsigned int texId;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	IplImage* texValues = cvCreateImage(cvSize(1024, 1024) , IPL_DEPTH_8U, 4);
	cvResize(voxe, texValues);

	// (2D, level 0, internal format, width, height, no border, format, pixel format, data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, texValues->imageData);

	cvReleaseImage(&texValues);

	LOG("Finished texture loading");
	return texId;
}

void Reconstruction::addSilhouette(Mat* silhouette, Mat* mvMatrix) {
	float xMinus = (x-1.f)/2;
	float yMinus = (y-1.f)/2;
	Mat projMat = (*cameraMatrix) * (*mvMatrix);
	LOG("Mult Projection Mat:");
	LOG(" %f %f %f %f", projMat.at<float>(0,0), projMat.at<float>(0,1), projMat.at<float>(0,2), projMat.at<float>(0,3));
	LOG(" %f %f %f %f", projMat.at<float>(1,0), projMat.at<float>(1,1), projMat.at<float>(1,2), projMat.at<float>(1,3));
	LOG(" %f %f %f %f", projMat.at<float>(2,0), projMat.at<float>(2,1), projMat.at<float>(2,2), projMat.at<float>(2,3));
	LOG(" %f %f %f %f", projMat.at<float>(3,0), projMat.at<float>(3,1), projMat.at<float>(3,2), projMat.at<float>(3,3));
	// Iterate through all voxels
	LOG("Iterating through Voxels.");
	LOG("Voxel nums. x: %d , y: %d , z: %d ", x, y, z);
	int count = 0;
	int countProj = 0;
	int countUnset = 0;
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < y; j++) {
			for (int k = 0; k < z; k++) {
				count++;
				if (voxels[i + j*x + k*y*x]) {
					float values[4] = {i-xMinus, j-yMinus, k, 1};
					Mat currVoxel(4,1, CV_32FC1, values);
					Mat proj = projMat * currVoxel;
					// Look at the position we projected to in the mask
					char mask = silhouette->at<char>(round(proj.at<float>(0,0)), round(proj.at<float>(1,0)));
					if (mask == GC_BGD || mask == GC_PR_BGD ) {
						LOG("Processed Voxel %d : Values %f, %f, %fb; projected to %f, %f, %f - unset", k + j*x + i*y*x, values[0], values[1], values[2], proj.at<float>(0,0), proj.at<float>(1,0), proj.at<float>(2,0));
						countUnset++;
						voxels[i + j*x + k*y*x] = false;
					}
					else {
						LOG("Processed Voxel %d : projected to %f, %f, %f - set", k + j*x + i*y*x, proj.at<float>(0,0), proj.at<float>(1,0), proj.at<float>(2,0));
					}
					countProj++;
				}
			}
		}
	}
	LOG("Projected all %d Voxels. Projected %d and unset %d of them.", count, countProj, countUnset);
}

char Reconstruction::getAt(int posX, int posY, int posZ) {
	int imgX = (posZ % nWidth) * x + posX;
	int imgY = (posZ / nWidth) * y + posY;
	return voxe->imageData[4*(imgX * voxe->width + imgY)];
}

void Reconstruction::setAt(int posX, int posY, int posZ, char value) {
	int imgX = (posZ % nWidth) * x + posX;
	int imgY = (posZ / nWidth) * y + posY;
	voxe->imageData[4*(imgX * voxe->width + imgY)] = value;
	voxe->imageData[4*(imgX * voxe->width + imgY)+1] = value;
	voxe->imageData[4*(imgX * voxe->width + imgY)+2] = value;
	voxe->imageData[4*(imgX * voxe->width + imgY)+3] = value;
}
