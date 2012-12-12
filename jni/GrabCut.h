#include <jni.h>
#include <android/log.h>
#include <algorithm>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QCAR/QCAR.h>
#include <QCAR/CameraDevice.h>
#include <QCAR/Image.h>
#include <QCAR/State.h>
#include <QCAR/Tool.h>
#include <QCAR/Trackable.h>
#include <QCAR/Frame.h>
#include <QCAR/Renderer.h>

#include "SampleUtils.h"

using namespace cv;
using namespace std;

#define LOG(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#ifndef LOG_TAG
#define LOG_TAG "GrabCut"
#endif

#ifndef GRABCUT_H
#define GRABCUT_H
class GrabCut {
public:
	GrabCut();
	~GrabCut();
	// Adds a list of coordinates to the foreground
	void addForegroundStroke(vector<Point>& fgdPixels);
	// Adds a list of coordinates to the background
	void addBackgroundStroke(vector<Point>& bgdPixels);
	// Initialize the grabCut
	void initRect(int left, int top, int right, int bottom);

	// Executes the actual GrabCut
	void executeGrabCut(int iterations);
	// Returns the image masked
	Mat* getMaskedImage();
	// Returns the mask
	Mat* getMask();
	// Returns the mask
	Mat* getMVMatrix();
	// Unsets the models to split the image
	void unsetModels();
	// Grab Frame
	void grabFrame();

	// Returns the height of the image
	int getHeight();
	// Returns the width of the image
	int getWidth();

private:
	Mat getFrame();
	Mat* getModelViewMat();
	Mat* mvMat;
	Mat frame, mask, lastMasked;
	Mat bgdModel, fgdModel;
	Rect rect;
};

#endif
