#include "GrabCut.h"

GrabCut::GrabCut(Mat image_in) : image(image_in) {
	mask.create(image.rows, image.cols, CV_8UC1);

}

void GrabCut::addForegroundStroke(vector<Point>& fgdPixels) {
	for (vector<Point>::const_iterator it = fgdPixels.begin(); it != fgdPixels.end(); it++) {
		mask.at<uchar>(*it) = GC_FGD;
	}

	bgdModel.release();
	fgdModel.release();
	grabCut( image, mask, rect, bgdModel, fgdModel, 0, GC_INIT_WITH_MASK );
}

void GrabCut::addBackgroundStroke(vector<Point>& bgdPixels) {
	for (vector<Point>::const_iterator it = bgdPixels.begin(); it != bgdPixels.end(); it++) {
		mask.at<uchar>(*it) = GC_BGD;
	}

	bgdModel.release();
	fgdModel.release();
	grabCut( image, mask, rect, bgdModel, fgdModel, 0, GC_INIT_WITH_MASK );
}

void GrabCut::executeGrabCut(int iterations) {
	grabCut( image, mask, rect, bgdModel, fgdModel, 1, GC_EVAL);
}
