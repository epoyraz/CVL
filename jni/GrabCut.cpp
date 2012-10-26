#include "GrabCut.h"

GrabCut::GrabCut(Mat image) : imageForSize(image) {
	mask.create(imageForSize.rows, imageForSize.cols, CV_8UC1);

}

void GrabCut::unsetModels() {
	bgdModel.release();
	fgdModel.release();
}

void GrabCut::addForegroundStroke(vector<Point>& fgdPixels) {
	for (vector<Point>::const_iterator it = fgdPixels.begin(); it != fgdPixels.end(); it++) {
		mask.at<uchar>(*it) = GC_FGD;
	}

	grabCut( imageForSize, mask, rect, bgdModel, fgdModel, 0, GC_INIT_WITH_MASK );
}

void GrabCut::addBackgroundStroke(vector<Point>& bgdPixels) {
	for (vector<Point>::const_iterator it = bgdPixels.begin(); it != bgdPixels.end(); it++) {
		mask.at<uchar>(*it) = GC_BGD;
	}

	grabCut( imageForSize, mask, rect, bgdModel, fgdModel, 0, GC_INIT_WITH_MASK );
}

void GrabCut::executeGrabCut(Mat& image, int iterations) {
	imageForSize = image;
	grabCut( image, mask, rect, bgdModel, fgdModel, 1, GC_EVAL);
}

Mat GrabCut::getMaskedImage() {
	return imageForSize.mul(mask);
}
