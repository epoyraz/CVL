/*
 * Reconstruction.cpp
 *
 *  Created on: 22.11.2012
 *      Author: gostefan
 */

#include "Reconstruction.h"

/**
 * Initializes all storages, gathers the camera matrix and calculates the texture dimensions
 * @param width_in The width of each frame image
 * @param height_in The height of each frame image
 * @param x_in The number of voxels in x direction
 * @param y_in The number of voxels in y direction
 * @param z_in The number of voxels in z direction
 */
Reconstruction::Reconstruction(int width_in, int height_in, int x_in, int y_in, int z_in) :
	x(x_in), y(y_in), z(z_in), imgWidth(width_in), imgHeight(height_in) {
	// Allocate the boolean storage for the voxels
	voxels = new bool[x * y * z];
	memset(voxels, (unsigned char)255, x*y*z*sizeof(bool));

	// Calculate how to distribute the texels on the texture
	calculateSizes();

	// Allocate the image to store the voxels on
	voxe = cvCreateImage(cvSize(width, height) , IPL_DEPTH_8U, 4);
	memset(voxe->imageData, (uchar) 255, width*height*4*sizeof(uchar));

	// Get the camera matrix for correct reconstruction
	getCameraMatrix();
	LOG("Initialized the reconstruction");
}

/**
 * Deletes all storages
 */
Reconstruction::~Reconstruction() {
	delete voxels;
	cvReleaseImage(&voxe);
	delete cameraMatrix->data;
	delete cameraMatrix;
}

/**
 * Gathers the camera matrix from vuforia and stores it locally
 */
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
}

/**
 * Calculates all needed data to allocate the texture image
 */
void Reconstruction::calculateSizes() {
	// Calculate how many pixels we need
	int prod = x * y * z;
	// Calculate the square root to get the best distribution on both axis
	width = ceil(sqrt((float)prod));
	// Get the number of images in first direction (makes the first dimension larger)
	nWidth = ceil((float)width/(float)x);
	width = nWidth * x;
	// Calculate the number of images in second direction
	nHeight = ceil((float)z/(float)nWidth);
	// Calculate height of texture
	height = nHeight * y;
}

/**
 * Returns an OpenGL texture with the current reconstruction state as content and scales it to an OpenGL conform size
 */
unsigned int Reconstruction::getTexture() {
	//LOG("Starting texture loading");
	unsigned int texId;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	IplImage* texValues = cvCreateImage(cvSize(1024, 1024) , IPL_DEPTH_8U, 4);
	cvResize(voxe, texValues);

	// (2D, level 0, internal format, width, height, no border, format, pixel format, data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, texValues->imageData);

	cvReleaseImage(&texValues);

	//LOG("Finished texture loading");
	return texId;
}

/**
 * Adds a silhouette to the reconstruction. This removes obsolete voxels and hereby improves the reconstruction
 * @param silhouette The silhouette that should be added
 * @param mvMatrix The model view matrix corresponding to the silhouette
 */
void Reconstruction::addSilhouette(Mat* silhouette, Mat* mvMatrix) {
	// Precompute stuff
	float xMinus = (x-1.f)/2;
	float yMinus = (y-1.f)/2;
	float heightDiv = imgHeight/2;
	float widthDiv = imgWidth/2;
	Mat projMat = (*cameraMatrix) * (*mvMatrix);

	// Iterate through all voxels
	LOG("Iterating through Voxels.");
	int count = 0;
	int countProj = 0;
	int countUnset = 0;
	for (int i = 0; i < x; i++) {
		for (int j = 0; j < y; j++) {
			for (int k = 0; k < z; k++) {
				count++;
				if (voxels[i + j*x + k*y*x]) {
					// Project the current voxel into screen coordinates
					float values[4] = {0.5*(i-xMinus), 0.5*(j-yMinus), 0.5*k, 1};
					Mat currVoxel(4,1, CV_32FC1, values);
					Mat proj = projMat * currVoxel;
					proj.at<float>(0,0) *= 320 / proj.at<float>(2,0);
					proj.at<float>(1,0) *= 240 / proj.at<float>(2,0);

					// Look at the position we projected to in the mask
					// This includes clipping to image size
					//LOG("%d: x %f y %f z %f ", k + j*z + i*y*z, proj.at<float>(1,0), proj.at<float>(0,0), proj.at<float>(2,0));
					int xCoord = round(-proj.at<float>(1,0) + heightDiv);
					int yCoord = round(proj.at<float>(0,0) + widthDiv);
					char mask = (char)0;
					if (xCoord >= 0 && xCoord < imgHeight && yCoord >= 0 && yCoord < imgWidth)
						mask = silhouette->at<char>(xCoord, yCoord);

					// If the voxel belongs to the background, unset it.
					if (mask == GC_BGD || mask == GC_PR_BGD ) {
						// Unset it in the boolean array
						countUnset++;
						voxels[i + j*x + k*y*x] = false;

						// Unset it in the texture
						int locationX = k%nWidth * x + i;
						int locationY = k/nWidth * y + j;
						int location = (locationX + locationY * x * nWidth)*4;
						voxe->imageData[location] = (char)0;
						voxe->imageData[location+1] = (char)0;
						voxe->imageData[location+2] = (char)0;
						voxe->imageData[location+3] = (char)0;
					}
					countProj++;
				}
			}
		}
	}
	LOG("Projected all %d Voxels. Projected %d and unset %d of them.", count, countProj, countUnset);
}

/**
 * Returns the number of slices in the texture in x direction
 * @return The number of slices in the texture in x direction
 */
float Reconstruction::getOverX() {
	return (float)nWidth;
}

/**
 * Returns the number of slices in the texture in y direction
 * @return The number of slices in the texture in y direction
 */
float Reconstruction::getOVerY() {
	return (float)nHeight;
}

/**
 * Returns the number of slices in the texture
 * @return The number of slices in the texture
 */
float Reconstruction::getNSlices() {
	return (float)z;
}
