/*
 * SilouhetteStorage.h
 *
 *  Created on: 21.11.2012
 *      Author: gostefan
 */

#ifndef SILOUHETTESTORAGE_H_
#define SILOUHETTESTORAGE_H_

#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

class SilhouetteStorage {
public:
	SilhouetteStorage();
	~SilhouetteStorage();
	void addSilhouette(Mat* silhouette, Mat* mvMat);
	Mat** getAllSilhouettes();
	Mat** getAllMVMatrices();
	int getNumSilhouettes();
private:
	vector<Mat*>* silhouettes;
	vector<Mat*>* mvMatrices;
};


#endif /* SILOUHETTESTORAGE_H_ */
