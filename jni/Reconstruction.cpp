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
	voxels = new bool[x * y * z];
	memset(voxels, (unsigned char)255, x*y*z*sizeof(bool));
	LOG("All voxels set true.");

	calculateSizes();
	LOG("Sizes calculated.");

	voxe = cvCreateImage(cvSize(width, height) , IPL_DEPTH_8U, 4);
	memset(voxe->imageData, (unsigned char) 255, x*y*z*4*sizeof(char));
	LOG("Image all voxels set true.");

}

void Reconstruction::calculateSizes() {
	// Calculate how many pixels we need
	int prod = x * y * z;
	// Calculate the square root to get the best distribution on both axis
	width = ceil(sqrt((float)prod));
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
}

unsigned int Reconstruction::getTexture() {
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

	return texId;
}

void Reconstruction::addSilhouette(Mat* silhouette, Mat* mvMatrix) {
	float xMinus = (x-1.f)/2;
	float yMinus = (y-1.f)/2;
	Mat mvMat = *mvMatrix;
	LOG("Rec working Projection Mat:");
	LOG(" %f %f %f %f", mvMat.at<float>(0,0), mvMat.at<float>(0,1), mvMat.at<float>(0,2), mvMat.at<float>(0,3));
	LOG(" %f %f %f %f", mvMat.at<float>(1,0), mvMat.at<float>(1,1), mvMat.at<float>(1,2), mvMat.at<float>(1,3));
	LOG(" %f %f %f %f", mvMat.at<float>(2,0), mvMat.at<float>(2,1), mvMat.at<float>(2,2), mvMat.at<float>(2,3));
	LOG(" %f %f %f %f", mvMat.at<float>(3,0), mvMat.at<float>(3,1), mvMat.at<float>(3,2), mvMat.at<float>(3,3));
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
					Mat proj = mvMat * currVoxel;
					// Look at the position we projected to in the mask
					if (silhouette->at<float>(round(proj.at<float>(0,0)), round(proj.at<float>(1,0))) <= 0) {
						//LOG("Processed Voxel %d : projected to %f, %f, %f - unset", i + j*x + k*y*x, proj.at<float>(0,0), proj.at<float>(1,0), proj.at<float>(2,0));
						countUnset++;
						voxels[i + j*x + k*y*x] = false;
					}
					else {
						//LOG("Processed Voxel %d : projected to %f, %f, %f - set", i + j*x + k*y*x, proj.at<float>(0,0), proj.at<float>(1,0), proj.at<float>(2,0));
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
