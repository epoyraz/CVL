
#include <jni.h>
#include <android/log.h>
#include "GrabCut.h"
#include "SilhouetteStorage.h"

GrabCut* grabCutObject;
SilhouetteStorage* sStorage = NULL;

extern "C"
{
	// Bridges over the fore-/background information from java
	JNIEXPORT void JNICALL
	Java_edu_ethz_s3d_GrabCutView_executeGrabCut(JNIEnv* env,jobject thiz,jfloatArray foreground,jfloatArray background, jint lFgd, jint lBgd) {

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

	JNIEXPORT void JNICALL Java_edu_ethz_s3d_GrabCutView_getMaskedFrame(JNIEnv* env, jobject, jlong addrFrame)
	{
		Mat* frame = (Mat*)addrFrame;
		Mat* mask = grabCutObject->getMaskedImage();
		cvtColor(*mask, *frame, CV_RGB2RGBA, 4);
	}
	JNIEXPORT jint JNICALL Java_edu_ethz_s3d_GrabCutView_getFrameHeight(JNIEnv* env, jobject)
	{
		return grabCutObject->getHeight();
	}
	JNIEXPORT jint JNICALL Java_edu_ethz_s3d_GrabCutView_getFrameWidth(JNIEnv* env, jobject)
	{
		return grabCutObject->getWidth();
	}
	JNIEXPORT void JNICALL Java_edu_ethz_s3d_GrabCutView_grabFrame(JNIEnv* env, jobject)
	{
		if (grabCutObject == NULL) {
			grabCutObject = new GrabCut();
		}
		grabCutObject->grabFrame();
	}
	JNIEXPORT void JNICALL Java_edu_ethz_s3d_GrabCutView_initGrabCut(JNIEnv* env,jobject thiz, jint left, jint top, jint right, jint bottom) {
		grabCutObject->initRect(left, top, right, bottom);
	}
	JNIEXPORT void JNICALL Java_edu_ethz_s3d_GrabCutView_moveToStorage(JNIEnv* env,jobject thiz) {
		if (sStorage == NULL) {
			sStorage = new SilhouetteStorage();
		}
		sStorage->addSilhouette(grabCutObject->getMask(), grabCutObject->getMVMatrix());
	}

}
