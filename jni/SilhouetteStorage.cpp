/*
 * SilhouetteStorage.cpp
 *
 *  Created on: 21.11.2012
 *      Author: gostefan
 */

#include "SilhouetteStorage.h"

SilhouetteStorage::SilhouetteStorage() {
	silhouettes = new vector<Mat*>();
	mvMatrices = new vector<Mat*>();
}

SilhouetteStorage::~SilhouetteStorage() {
	delete silhouettes;
	delete mvMatrices;
}

void SilhouetteStorage::addSilhouette(Mat* silhouette, Mat* mvMat) {
	silhouettes->push_back(silhouette);
	mvMatrices->push_back(mvMat);
	LOG("Stored Projection Mat:");
	LOG(" %f %f %f %f", mvMat->at<float>(0,0), mvMat->at<float>(0,1), mvMat->at<float>(0,2), mvMat->at<float>(0,3));
	LOG(" %f %f %f %f", mvMat->at<float>(1,0), mvMat->at<float>(1,1), mvMat->at<float>(1,2), mvMat->at<float>(1,3));
	LOG(" %f %f %f %f", mvMat->at<float>(2,0), mvMat->at<float>(2,1), mvMat->at<float>(2,2), mvMat->at<float>(2,3));
	LOG(" %f %f %f %f", mvMat->at<float>(3,0), mvMat->at<float>(3,1), mvMat->at<float>(3,2), mvMat->at<float>(3,3));
}

Mat** SilhouetteStorage::getAllSilhouettes() {
	return &silhouettes->at(0);
}

Mat** SilhouetteStorage::getAllMVMatrices() {
	return &mvMatrices->at(0);
}

Mat* SilhouetteStorage::getLastSilhouette() {
	return silhouettes->back();
}

Mat* SilhouetteStorage::getLastMVMatrice() {
	Mat* mvMat = mvMatrices->back();
	LOG("SS Returned Projection Mat:");
	LOG(" %f %f %f %f", mvMat->at<float>(0,0), mvMat->at<float>(0,1), mvMat->at<float>(0,2), mvMat->at<float>(0,3));
	LOG(" %f %f %f %f", mvMat->at<float>(1,0), mvMat->at<float>(1,1), mvMat->at<float>(1,2), mvMat->at<float>(1,3));
	LOG(" %f %f %f %f", mvMat->at<float>(2,0), mvMat->at<float>(2,1), mvMat->at<float>(2,2), mvMat->at<float>(2,3));
	LOG(" %f %f %f %f", mvMat->at<float>(3,0), mvMat->at<float>(3,1), mvMat->at<float>(3,2), mvMat->at<float>(3,3));
	return mvMatrices->back();
}

int SilhouetteStorage::getNumSilhouettes() {
	return silhouettes->size();
}
