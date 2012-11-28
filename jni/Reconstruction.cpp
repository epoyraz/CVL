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
	LOG("Set all voxels.");
	// TODO: In the above statement possibly 7 values are missed.
}

Reconstruction::~Reconstruction() {
	delete voxels;
}

unsigned int Reconstruction::getTexture() {
	unsigned int texId;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);*/
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
					Point at(round(proj.at<float>(0,0)), round(proj.at<float>(1,0)));
					if (silhouette->at<float>(at) <= 0) {
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
