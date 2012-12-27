/*
 * SilouhetteStorage.h
 *
 *  Created on: 21.11.2012
 *      Author: gostefan
 */

#ifndef SILOUHETTESTORAGE_H_
#define SILOUHETTESTORAGE_H_

#include <android/log.h>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

#define LOG(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#ifndef LOG_TAG
#define LOG_TAG "SilhouetteStorage"
#endif

class SilhouetteStorage {
public:
	SilhouetteStorage();
	~SilhouetteStorage();
	void addSilhouette(Mat* silhouette, Mat* mvMat);
	Mat** getAllSilhouettes();
	Mat** getAllMVMatrices();
	Mat* getLastSilhouette();
	Mat* getLastMVMatrice();
	int getNumSilhouettes();
private:
	/**
	 * The storage list for the silhouettes
	 */
	vector<Mat*>* silhouettes;
	/**
	 * The storage list for the model view matrices
	 */
	vector<Mat*>* mvMatrices;
};


#endif /* SILOUHETTESTORAGE_H_ */
