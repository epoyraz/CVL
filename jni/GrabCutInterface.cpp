
#include <jni.h>
#include <android/log.h>
#include "GrabCut.h"
#include "SilhouetteStorage.h"

/**
 * The actual instance of the GrabCut class
 */
GrabCut* grabCutObject;
/**
 * The actual instance of the Silhouette Storage
 */
SilhouetteStorage* sStorage = NULL;

extern "C"
{
	/**
	 * Adds the fore-/background strokes to the grab cut and executes it
	 */
	JNIEXPORT void JNICALL Java_edu_ethz_s3d_GrabCutView_executeGrabCut(JNIEnv* env,jobject thiz,jfloatArray foreground,jfloatArray background, jint lFgd, jint lBgd) {

	  LOG("Java_edu_ethz_s3d_GrabCutView_executeGrabCut");
	  float* fgdPos = env->GetFloatArrayElements(foreground,0);

	  float* bgdPos = env->GetFloatArrayElements(background,0);

	  vector<Point> fgdPixels;
	  vector<Point> bgdPixels;

	  for (int i=0; i < lFgd; i++) {
		  LOG("Iteration: %d ", i);
		  fgdPixels.push_back(Point(fgdPos[2*i], fgdPos[2*i+1]));
	  }

	  for (int i=0; i < lBgd; i++) {
		  LOG("Iteration: %d ", i);
		  bgdPixels.push_back(Point(bgdPos[2*i], bgdPos[2*i+1]));
	  }


	  grabCutObject->addForegroundStroke(fgdPixels);
	  grabCutObject->addBackgroundStroke(bgdPixels);

	  LOG("Run Grab");
	  grabCutObject->executeGrabCut(1);

	  env->ReleaseFloatArrayElements(foreground, fgdPos,0);
	  env->ReleaseFloatArrayElements(background, bgdPos,0);
	}

	/**
	 * Converts and copies the frame data into the Java OpenCV matrix
	 */
	JNIEXPORT void JNICALL Java_edu_ethz_s3d_GrabCutView_getMaskedFrame(JNIEnv* env, jobject, jlong addrFrame)
	{
		Mat* frame = (Mat*)addrFrame;
		Mat* mask = grabCutObject->getMaskedImage();
		cvtColor(*mask, *frame, CV_RGB2RGBA, 4);
	}

	/**
	 * Converts and copies the mask data into the Java OpenCV matrix
	 */
	JNIEXPORT void JNICALL Java_edu_ethz_s3d_GrabCutView_getMask(JNIEnv* env, jobject, jlong addrFrame)
	{
		Mat* frame = (Mat*)addrFrame;
		Mat* mask = grabCutObject->getScreenMask();
		cvtColor(*mask, *frame, CV_GRAY2RGBA, 4);
	}

	/**
	 * Returns the number of pixels in the height of the frame to Java
	 */
	JNIEXPORT jint JNICALL Java_edu_ethz_s3d_GrabCutView_getFrameHeight(JNIEnv* env, jobject)
	{
		return grabCutObject->getHeight();
	}

	/**
	 * Returns the number of pixels in the width of the frame to Java
	 */
	JNIEXPORT jint JNICALL Java_edu_ethz_s3d_GrabCutView_getFrameWidth(JNIEnv* env, jobject)
	{
		return grabCutObject->getWidth();
	}

	/**
	 * If the GrabCut object is not yet initialized it initializes it and tells it to get the current frame
	 */
	JNIEXPORT void JNICALL Java_edu_ethz_s3d_GrabCutView_grabFrame(JNIEnv* env, jobject)
	{
		if (grabCutObject == NULL) {
			grabCutObject = new GrabCut();
		}
		grabCutObject->grabFrame();
	}

	/**
	 * Initializes the GrabCut with the given rectangle coordinates
	 */
	JNIEXPORT void JNICALL Java_edu_ethz_s3d_GrabCutView_initGrabCut(JNIEnv* env,jobject thiz, jint left, jint top, jint right, jint bottom) {
		grabCutObject->initRect(left, top, right, bottom);
	}

	/**
	 * Initializes the SilhouetteStorage object if not existent and stores the current mask into it
	 */
	JNIEXPORT void JNICALL Java_edu_ethz_s3d_GrabCutView_moveToStorage(JNIEnv* env,jobject thiz) {
		if (sStorage == NULL) {
			sStorage = new SilhouetteStorage();
		}
		sStorage->addSilhouette(grabCutObject->getMask(), grabCutObject->getMVMatrix());
	}

}
