#include "GrabCut.h"

GrabCut::GrabCut() {
	lastMasked = getFrame();
	mask.create(lastMasked.rows, lastMasked.cols, CV_8UC1);
}

GrabCut::~GrabCut() {
}

void GrabCut::unsetModels() {
	bgdModel.release();
	fgdModel.release();
}

void GrabCut::addForegroundStroke(vector<Point>& fgdPixels) {
	LOG("Adding foreground Stroke");
	for (vector<Point>::const_iterator it = fgdPixels.begin(); it != fgdPixels.end(); it++) {
		LOG("Mask was %d ", mask.at<uchar>(*it));
		mask.at<uchar>(*it) = GC_FGD;
	}
	LOG("Added foreground Stroke");
}

void GrabCut::addBackgroundStroke(vector<Point>& bgdPixels) {
	LOG("Adding background Stroke");
	for (vector<Point>::const_iterator it = bgdPixels.begin(); it != bgdPixels.end(); it++) {
		LOG("Mask was %d ", mask.at<uchar>(*it));
		mask.at<uchar>(*it) = GC_BGD;
	}
	LOG("Added background Stroke");
}

void GrabCut::executeGrabCut(int iterations) {
	LOG("Executing GrabCut");
	grabCut(frame, mask, rect, bgdModel, fgdModel, 0, GC_INIT_WITH_MASK );
	LOG("Initialized GrabCut");
	grabCut(frame, mask, rect, bgdModel, fgdModel, 1);
	LOG("Executed GrabCut");
	for (int i = 0; i < frame.rows; i++) {
		for (int j = 0; j < frame.cols; j++) {
			for (int d = 0; d < 3; d++) {
				lastMasked.data[i*3*frame.cols+j*3+d] = frame.data[i*3*frame.cols+j*3+d] * mask.data[i*frame.cols+j];
			}
		}
	}
	LOG("Created Frame");
}

void GrabCut::grabFrame() {
	frame = getFrame();
	mask = Mat(frame.size(), CV_8UC1, GC_EVAL);
	lastMasked = frame;
}

Mat* GrabCut::getMaskedImage() {
	return &lastMasked;
}

Mat GrabCut::getFrame() {
	QCAR::State state = QCAR::Renderer::getInstance().begin();

	QCAR::Frame frame = state.getFrame();
	for (int i = 0; i < frame.getNumImages(); i++)
	{
	    const QCAR::Image *qcarImage = frame.getImage(i);
	    if (qcarImage->getFormat() == QCAR::RGB888)
	    {
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
