#include "GrabCut.h"

GrabCut::GrabCut() {
	mvMat = NULL;
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
						lastMaskedV[d] = frameV[d];
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
	LOG("Projection Mat:");
	LOG(" %f %f %f %f", mvMat->at<float>(0,0), mvMat->at<float>(0,1), mvMat->at<float>(0,2), mvMat->at<float>(0,3));
	LOG(" %f %f %f %f", mvMat->at<float>(1,0), mvMat->at<float>(1,1), mvMat->at<float>(1,2), mvMat->at<float>(1,3));
	LOG(" %f %f %f %f", mvMat->at<float>(2,0), mvMat->at<float>(2,1), mvMat->at<float>(2,2), mvMat->at<float>(2,3));
	LOG(" %f %f %f %f", mvMat->at<float>(3,0), mvMat->at<float>(3,1), mvMat->at<float>(3,2), mvMat->at<float>(3,3));
	frame = getFrame();
	LOG("Loaded Frame");
	frame.copyTo(lastMasked);
}

Mat* GrabCut::getMaskedImage() {
	return &lastMasked;
}

Mat* GrabCut::getMask() {
	char* maskData = new char[mask.cols*mask.rows];
	memcpy(maskData, mask.data, mask.cols*mask.rows*sizeof(char));
	Mat* maskCopy = new Mat(mask.cols, mask.rows, CV_8UC3, maskData);
	return maskCopy;
}

Mat* GrabCut::getMVMatrix() {
	LOG("Returned Projection Mat:");
	LOG(" %f %f %f %f", mvMat->at<float>(0,0), mvMat->at<float>(0,1), mvMat->at<float>(0,2), mvMat->at<float>(0,3));
	LOG(" %f %f %f %f", mvMat->at<float>(1,0), mvMat->at<float>(1,1), mvMat->at<float>(1,2), mvMat->at<float>(1,3));
	LOG(" %f %f %f %f", mvMat->at<float>(2,0), mvMat->at<float>(2,1), mvMat->at<float>(2,2), mvMat->at<float>(2,3));
	LOG(" %f %f %f %f", mvMat->at<float>(3,0), mvMat->at<float>(3,1), mvMat->at<float>(3,2), mvMat->at<float>(3,3));
	return mvMat;
}

Mat* GrabCut::getModelViewMat() {
	LOG("Getting MV-Matrix");
	QCAR::State state = QCAR::Renderer::getInstance().begin();
	// Only get something meaningful if exactly one trackable is tracked
	if (state.getNumActiveTrackables() == 1) {
		// Get the model-view matrix
		// First we have to get the trackable
		const QCAR::Trackable* trackable = state.getActiveTrackable(0);
		QCAR::Matrix44F modelViewMatrix = QCAR::Tool::convertPose2GLMatrix(trackable->getPose());

		// We need to allocate the data on the heap
		float* data = new float[16];
		for (int i = 0; i < 4*4; i++) {
			data[i] = modelViewMatrix.data[i];
		}
		LOG ("Allocated Memory for mv-Matrix and copied data.");

		// Create matrix
		return new Mat(4, 4, DataType<float>::type, data);
	}
	return new Mat();
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
