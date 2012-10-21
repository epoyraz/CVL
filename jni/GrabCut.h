#include <jni.h>
#include <algorithm>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

class GrabCut {
public:
	// Adds a list of coordinates to the foreground
	void addForegroundStroke(int coordinates[][2]);
	// Adds a list of coordinates to the background
	void addBackgroundStroke(int coordinates[][2]);

	// Executes the actual GrabCut
	void executeGrabCut(int iterations);

private:
	Mat image, mask;
	Mat bgdModel, fgdModel;
	Rect rect;
};
