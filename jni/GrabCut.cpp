#include "GrabCut.h"

GrabCut::GrabCut() {
}

GrabCut::~GrabCut() {
}

void GrabCut::unsetModels() {
	bgdModel.release();
	fgdModel.release();
}

void GrabCut::addForegroundStroke(vector<Point>& fgdPixels) {
	for (vector<Point>::const_iterator it = fgdPixels.begin(); it != fgdPixels.end(); it++) {
		//LOG("Changing at %d, %d ", (*it).x, (*it).y);
		mask.at<uchar>(*it) = GC_FGD;
	}
}

void GrabCut::addBackgroundStroke(vector<Point>& bgdPixels) {
	for (vector<Point>::const_iterator it = bgdPixels.begin(); it != bgdPixels.end(); it++) {
		//LOG("Changing at %d, %d ", (*it).x, (*it).y);
		mask.at<uchar>(*it) = GC_BGD;
	}
}

void GrabCut::executeGrabCut(int iterations) {
	LOG("Executing GrabCut");
	grabCut(frame, mask, rect, bgdModel, fgdModel, 1, GC_EVAL);
	LOG("Executed GrabCut");
	// Iterate through all pixels and set fore/background
	for (int i = 0; i < frame.rows; i++) {
		for (int j = 0; j < frame.cols; j++) {
			// Extract pixel values
			Vec3b frameV = frame.at<Vec3b>(i, j);
			Vec3b lastMaskedV = lastMasked.at<Vec3b>(i, j);
			char maskV = mask.at<char>(i, j);

			// Iterate through channels
			for (int d = 0; d < 3; d++) {
				switch (maskV) {
					case GC_FGD:
					case GC_PR_FGD:
						break;
					default:
						lastMaskedV[d] = frameV[d] / 2;
						break;
				}
			}

			// Save values back
			lastMasked.at<Vec3b>(i, j) = lastMaskedV;
		}
	}
	LOG("Created Frame");
}

void GrabCut::grabFrame() {
	mvMat = getModelViewMat();
	frame = getFrame();
	LOG("Loaded Frame");
	frame.copyTo(lastMasked);
}

Mat* GrabCut::getMaskedImage() {
	return &lastMasked;
}

Mat* GrabCut::getMask() {
	return &mask;
}

Mat* GrabCut::getMVMatrix() {
	return &mvMat;
}

Mat GrabCut::getModelViewMat() {
	LOG("Getting MV-Matrix");
	QCAR::State state = QCAR::Renderer::getInstance().begin();
	// Only get something meaningful if exactly one trackable is tracked
	if (state.getNumActiveTrackables() == 1) {
		// Get the model-view matrix
		// First we have to get the trackable
		const QCAR::Trackable* trackable = state.getActiveTrackable(0);
		QCAR::Matrix44F modelViewMatrix = QCAR::Tool::convertPose2GLMatrix(trackable->getPose());
		return Mat(4, 4, CV_32FC1, modelViewMatrix.data);
	}
	return Mat();
}

Mat GrabCut::getFrame() {
	LOG("Getting Frame");
	QCAR::State state = QCAR::Renderer::getInstance().begin();
	// Get the frame data
	QCAR::Frame frame = state.getFrame();
	for (int i = 0; i < frame.getNumImages(); i++)
	{
		const QCAR::Image *qcarImage = frame.getImage(i);
		if (qcarImage->getFormat() == QCAR::RGB888)
		{
			// Create a matrix with 3 channels, the image height and width
			return Mat(qcarImage->getHeight(), qcarImage->getWidth(), CV_8UC3, (unsigned char *) const_cast<void*>(qcarImage->getPixels()));
		}
	}
	return Mat();
}

int GrabCut::getHeight() {
	return frame.rows;
}

int GrabCut::getWidth() {
	return frame.cols;
}

void GrabCut::initRect(int left, int top, int right, int bottom) {
	rect = Rect(Point(left, top), Point(right, bottom));
	LOG("Rect GrabCut");
	grabCut(frame, mask, rect, bgdModel, fgdModel, 5, GC_INIT_WITH_RECT );
	executeGrabCut(1);
}
