#include "GrabCut.h"

void GrabCut::addForegroundStroke(int coordinates[][2]) {

}

void GrabCut::addBackgroundStroke(int coordinates[][2]) {

}

void GrabCut::executeGrabCut(int iterations) {
	grabCut( image, mask, rect, bgdModel, fgdModel, 0, GC_INIT_WITH_RECT );
}
