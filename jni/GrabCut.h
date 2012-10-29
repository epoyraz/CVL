#include <jni.h>
#include <android/log.h>
#include <algorithm>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QCAR/QCAR.h>
#include <QCAR/CameraDevice.h>
#include <QCAR/Image.h>
#include <QCAR/State.h>
#include <QCAR/Frame.h>
#include <QCAR/Renderer.h>

using namespace cv;
using namespace std;

#define LOG(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#ifndef LOG_TAG
#define LOG_TAG "GrabCut"
#endif

class GrabCut {
public:
	GrabCut();
	// Adds a list of coordinates to the foreground
	void addForegroundStroke(vector<Point>& fgdPixels);
	// Adds a list of coordinates to the background
	void addBackgroundStroke(vector<Point>& bgdPixels);

	// Executes the actual GrabCut
	void executeGrabCut(Mat& image, int iterations);
	// Returns the image masked
	Mat getMaskedImage();
	// Unsets the models to split the image
	void unsetModels();

	// Returns the height of the image
	int getHeight();
	// Returns the width of the image
	int getWidth();
	Mat getFrame();

private:
	Mat imageForSize, mask;
	Mat bgdModel, fgdModel;
	Rect rect;
};
