#include "GrabCut.h"

/**
 * The scale between the internal coordinate system and the vuforia coordinate system
 * Also defined in ImageTargets.cpp
 */
const float kObjectScale = 3.f;

GrabCut::GrabCut() {
	mvMat = NULL;
}

GrabCut::~GrabCut() {
}

/**
 * Unsets the models for fore- and background
 * Currently it is unused
 */
void GrabCut::unsetModels() {
	bgdModel.release();
	fgdModel.release();
}

/**
 * Adds the contents of a vector of points to the foreground
 * @param bgdPixels The vector of points which are surely foreground
 */
void GrabCut::addForegroundStroke(vector<Point>& fgdPixels) {
	for (vector<Point>::const_iterator it = fgdPixels.begin(); it != fgdPixels.end(); it++) {
		//LOG("Changing at %d, %d ", (*it).x, (*it).y);
		mask.at<uchar>(*it) = GC_FGD;
	}
}

/**
 * Adds the contents of a vector of points to the background
 * @param bgdPixels The vector of points which are surely background
 */
void GrabCut::addBackgroundStroke(vector<Point>& bgdPixels) {
	for (vector<Point>::const_iterator it = bgdPixels.begin(); it != bgdPixels.end(); it++) {
		//LOG("Changing at %d, %d ", (*it).x, (*it).y);
		mask.at<uchar>(*it) = GC_BGD;
	}
}

/**
 * Executes the GrabCut with the given number of iterations
 * @param iterations The number of iterations to do
 */
void GrabCut::executeGrabCut(int iterations) {
	// Actually compute the grabCut
	grabCut(frame, mask, rect, bgdModel, fgdModel, 1, GC_EVAL);
	// Copy the mask screen mask
	mask.copyTo(screenMask);
	// Iterate through all pixels and set fore/background
	for (int i = 0; i < frame.rows; i++) {
		for (int j = 0; j < frame.cols; j++) {
			// Extract pixel values
			Vec3b frameV = frame.at<Vec3b>(i, j);
			Vec3b lastMaskedV = lastMasked.at<Vec3b>(i, j);
			char screen = screenMask.at<char>(i, j);
			char maskV = mask.at<char>(i, j);

			// Iterate through channels
			for (int d = 0; d < 3; d++) {
				switch (maskV) {
					case GC_FGD:
					case GC_PR_FGD:
						lastMaskedV[d] = frameV[d];
						screen = (char)255;
						break;
					default:
						lastMaskedV[d] = frameV[d] / 2;
						screen = (char)0;
						break;
				}
			}

			// Save values back
			lastMasked.at<Vec3b>(i, j) = lastMaskedV;
			screenMask.at<char>(i, j) = screen;
		}
	}
	LOG("Executed GrabCut and created frame");
}

/**
 * Loads the frame and model view matrix
 */
void GrabCut::grabFrame() {
	mvMat = getModelViewMat();
	frame = getFrame();
	frame.copyTo(lastMasked);
	LOG("Loaded Frame and model view matrix:");
}

/**
 * Returns the masked image pointer
 * @return The pointer to the last masked image
 */
Mat* GrabCut::getMaskedImage() {
	return &lastMasked;
}

/**
 * Returns the mask pointer
 * @return The pointer to the last mask
 */
Mat* GrabCut::getMask() {
	return &mask;
}

/**
 * Returns the pointer to the mask for displaying
 * @return The pointer to the mask for displaying
 */
Mat* GrabCut::getScreenMask() {
	return &screenMask;
}

/**
 * Returns the pointer to the model view matrix
 * @return The pointer to the model view matrix
 */
Mat* GrabCut::getMVMatrix() {
	return mvMat;
}

/**
 * Loads the model view matrix from vuforia
 * It also converts it from OpenGL matrix format to OpenCV format
 * @return The pointer to the model view matrix
 */
Mat* GrabCut::getModelViewMat() {
	QCAR::State state = QCAR::Renderer::getInstance().begin();
	// Only get something meaningful if exactly one trackable is tracked
	if (state.getNumActiveTrackables() == 1) {
		// Get the model-view matrix
		// First we have to get the trackable
		const QCAR::Trackable* trackable = state.getActiveTrackable(0);
		QCAR::Matrix44F modelViewMatrix = QCAR::Tool::convertPose2GLMatrix(trackable->getPose());
        SampleUtils::translatePoseMatrix(0.0f, 0.0f, kObjectScale,
                                         &modelViewMatrix.data[0]);
        SampleUtils::scalePoseMatrix(kObjectScale, kObjectScale, kObjectScale,
                                     &modelViewMatrix.data[0]);

		// We need to allocate the data on the heap
		float* data = new float[16];
		for (int i = 0; i < 4*4; i++) {
			data[i] = modelViewMatrix.data[(i%4)*4+(i/4)];
		}
		LOG ("Gathered the model view matrix from vuforia");

		// Create matrix
		return new Mat(4, 4, DataType<float>::type, data);
	}
	return new Mat();
}

/**
 * Loads the frame data from vuforia
 * @return Returns a matrix with the frame data stored in it
 */
Mat GrabCut::getFrame() {
	QCAR::State state = QCAR::Renderer::getInstance().begin();
	// Get the frame data
	QCAR::Frame frame = state.getFrame();
	for (int i = 0; i < frame.getNumImages(); i++)
	{
		const QCAR::Image *qcarImage = frame.getImage(i);
		if (qcarImage->getFormat() == QCAR::RGB888)
		{
			LOG("Gathered the frame data from vuforia");
			// Create a matrix with 3 channels, the image height and width
			return Mat(qcarImage->getHeight(), qcarImage->getWidth(), CV_8UC3, (unsigned char *) const_cast<void*>(qcarImage->getPixels()));
		}
	}
	return Mat();
}

/**
 * Returns the height of the last captured frame
 * @return The number of pixels in the height of the frame
 */
int GrabCut::getHeight() {
	return frame.rows;
}

/**
 * Returns the width of the last captured frame
 * @return The number of pixels in the width of the frame
 */
int GrabCut::getWidth() {
	return frame.cols;
}

/**
 * Initiates the GrabCut with a rectangle
 * @param left The distance of the left coordinate
 * @param top The distance of the top coordinate
 * @param right The distance of the right coordinate (from the left border)
 * @param bottom The distance of the bottom coordinate (from the top border)
 */
void GrabCut::initRect(int left, int top, int right, int bottom) {
	rect = Rect(Point(left, top), Point(right, bottom));
	LOG("Rect GrabCut");
	grabCut(frame, mask, rect, bgdModel, fgdModel, 5, GC_INIT_WITH_RECT );
	executeGrabCut(1);
}
