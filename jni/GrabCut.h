#include <jni.h>
#include <algorithm>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

class GrabCut {
public:
	GrabCut(Mat image);
	// Adds a list of coordinates to the foreground
	void addForegroundStroke(vector<Point>& fgdPixels);
	// Adds a list of coordinates to the background
	void addBackgroundStroke(vector<Point>& bgdPixels);

	// Executes the actual GrabCut
	void executeGrabCut(int iterations);

private:
	Mat image, mask;
	Mat bgdModel, fgdModel;
	Rect rect;
};
