/*
 * SilhouetteStorage.cpp
 *
 *  Created on: 21.11.2012
 *      Author: gostefan
 */

#include "SilhouetteStorage.h"

/**
 * Allocates the lists to store silhouettes and model view matrices
 */
SilhouetteStorage::SilhouetteStorage() {
	silhouettes = new vector<Mat*>();
	mvMatrices = new vector<Mat*>();
}

/**
 * This deletes the silhouette list and all the model view matrices and their list.
 * The silhouettes are not allocated using new and hence don't need to bee deleted
 */
SilhouetteStorage::~SilhouetteStorage() {
	delete silhouettes;
	for (vector<Mat*>::iterator iter = mvMatrices->begin(); iter != mvMatrices->end(); iter++) {
		delete (*iter)->data;
		delete (*iter);
	}
	delete mvMatrices;
}

/**
 * Adds a slihouette image and the corresponding model view matrix to the storage
 * @param silhouette The silhouette that should be added
 * @param mvMat The model view matrix that corresponds to the silhouette added
 */
void SilhouetteStorage::addSilhouette(Mat* silhouette, Mat* mvMat) {
	silhouettes->push_back(silhouette);
	mvMatrices->push_back(mvMat);
}

/**
 * Returns the beginning of the silhouette list
 * This is intended for a more in-depth reconstruction of certain parts
 * Until now it is unused
 * @return A pointer to the first matrix pointer
 */
Mat** SilhouetteStorage::getAllSilhouettes() {
	return &silhouettes->at(0);
}

/**
 * Returns the beginning of the model view matrix list
 * This is intended for a more in-depth reconstruction of certain parts
 * Until now it is unused
 * @return A pointer to the first matrix pointer
 */
Mat** SilhouetteStorage::getAllMVMatrices() {
	return &mvMatrices->at(0);
}

/**
 * Returns the last inserted Silhouette
 * @return The pointer to the last inserted silhouette
 */
Mat* SilhouetteStorage::getLastSilhouette() {
	return silhouettes->back();
}

/**
 * Returns the last inserted model view matrix
 * Sorry for the bad spelling
 * @return The pointer to the last inserted matrix
 */
Mat* SilhouetteStorage::getLastMVMatrice() {
	Mat* mvMat = mvMatrices->back();
	return mvMatrices->back();
}

/**
 * Returns how many silhouettes and model view matrices are stored
 * This is intended for a more in-depth reconstruction of certain parts
 * Until now it is unused
 * @return How many silhouettes and model view matrices are stored
 */
int SilhouetteStorage::getNumSilhouettes() {
	return silhouettes->size();
}
