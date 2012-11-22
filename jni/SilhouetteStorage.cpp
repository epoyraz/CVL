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
}

Mat** SilhouetteStorage::getAllSilhouettes() {
	return &silhouettes->at(0);
}

Mat** SilhouetteStorage::getAllMVMatrices() {
	return &mvMatrices->at(0);
}

int SilhouetteStorage::getNumSilhouettes() {
	return silhouettes->size();
}
