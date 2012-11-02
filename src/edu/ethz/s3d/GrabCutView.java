package edu.ethz.s3d;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedList;

import org.opencv.android.Utils;
import org.opencv.core.CvType;
import org.opencv.core.Mat;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.view.MotionEvent;
import android.view.MotionEvent.PointerCoords;
import android.view.View;
import android.view.View.OnTouchListener;
import android.widget.ImageView;

public class GrabCutView extends ImageView implements OnTouchListener {
	
	private LinkedList<LinkedList<MotionEvent.PointerCoords>> fgdStrokes;
	private LinkedList<LinkedList<MotionEvent.PointerCoords>> bgdStrokes;
	public boolean isForeground = true;
	private boolean hasFgd = false;
	private boolean hasBgd = false;
	private Paint fgdColor = new Paint();
	private Paint bgdColor = new Paint();

	public GrabCutView(Context context) {
		super(context);
		
		// Set everything so the drawing will work
        setOnTouchListener(this);
        //this.setWillNotDraw(false);
        
        // Initialize drawing stuff
        fgdStrokes = new LinkedList<LinkedList<MotionEvent.PointerCoords>>();
        bgdStrokes = new LinkedList<LinkedList<MotionEvent.PointerCoords>>();
        fgdColor.setColor(Color.BLUE);
        fgdColor.setStrokeWidth(10);
        bgdColor.setColor(Color.GREEN);
        bgdColor.setStrokeWidth(10);
        
        // Initialize Image
		grabFrame();
		setScaleType(ScaleType.CENTER_CROP);
        updateFrame();
	}
	
    public boolean onTouch(View v, MotionEvent event) {
		DebugLog.LOGD("S3DView::onTouch");
		// Select corresponding list
		LinkedList<LinkedList<MotionEvent.PointerCoords>> outerList = isForeground ? fgdStrokes : bgdStrokes;
		hasFgd = isForeground ? true : hasFgd;
		hasBgd = !isForeground ? true : hasBgd;
		// Initialize coordinates object
		MotionEvent.PointerCoords coordinates = new MotionEvent.PointerCoords();
		
		// Switch the different event types
		switch (event.getAction()) {
			case MotionEvent.ACTION_DOWN:
				// We store the first position of the touch by generating a new list
				event.getPointerCoords(0, coordinates);
				LinkedList<MotionEvent.PointerCoords> innerList = new LinkedList<MotionEvent.PointerCoords>();
				innerList.add(coordinates);
				outerList.add(innerList);
				break;
			case MotionEvent.ACTION_MOVE:
				// We get the latest position information
				event.getPointerCoords(0, coordinates);
				outerList.getLast().add(coordinates);
				
				// Drawing is done by the onDraw function
				break;
			case MotionEvent.ACTION_UP:
				// We finish the tracking of points and submit the array to native code
				float[] fgdCoords = convertToArray(fgdStrokes);
				float[] bgdCoords = convertToArray(bgdStrokes);
				if (hasFgd && hasBgd) {
					executeGrabCut(fgdCoords, bgdCoords, fgdCoords.length, bgdCoords.length);
					updateFrame();
				}
				break;
					
		}
		invalidate();
		// We want to get all follow-up events.
		return true;
	}

    private float[] convertToArray(
			LinkedList<LinkedList<PointerCoords>> strokes) {
    	ArrayList<Float> list = new ArrayList<Float>(); 
    	Iterator<LinkedList<MotionEvent.PointerCoords>> outerIter = strokes.iterator();
    	while (outerIter.hasNext()) {
    		Iterator<MotionEvent.PointerCoords> innerIter = outerIter.next().iterator();
    		while (innerIter.hasNext()) {
    			MotionEvent.PointerCoords coord = innerIter.next();
    			list.add(coord.x);
    			list.add(coord.y);
    		}
    	}
    	float[] finalArray = new float[list.size()];
    	Iterator<Float> iter = list.iterator();
    	int i = 0;
    	while (iter.hasNext()) {
    		finalArray[i] = iter.next();
    		i++;
    	}
		return finalArray;
	}

	@Override
    public void onDraw(Canvas canvas) {
		super.onDraw(canvas);
        DebugLog.LOGD("S3DView::onDraw");
    	
    	// Draw foreground strokes
    	Iterator<LinkedList<MotionEvent.PointerCoords>> iter = fgdStrokes.iterator();
    	while (iter.hasNext()) {
    		Iterator<MotionEvent.PointerCoords> innerIter = iter.next().iterator();
    		MotionEvent.PointerCoords lastCoords = innerIter.next();
    		MotionEvent.PointerCoords currCoords = null;
    		while (innerIter.hasNext()) {
    			currCoords = innerIter.next();
    			canvas.drawLine(lastCoords.x, lastCoords.y, currCoords.x, currCoords.y, fgdColor);
    			lastCoords = currCoords;
    		}
    	}
    	
    	// Draw background strokes
    	iter = bgdStrokes.iterator();
    	while (iter.hasNext()) {
    		Iterator<MotionEvent.PointerCoords> innerIter = iter.next().iterator();
    		MotionEvent.PointerCoords lastCoords = innerIter.next();
    		MotionEvent.PointerCoords currCoords = null;
    		while (innerIter.hasNext()) {
    			currCoords = innerIter.next();
    			canvas.drawLine(lastCoords.x, lastCoords.y, currCoords.x, currCoords.y, bgdColor);
    			lastCoords = currCoords;
    		}
    	}
    }
	
	protected native void getMaskedFrame(long address);
	protected native int getFrameHeight();
	protected native int getFrameWidth();
	protected native void grabFrame();
	protected native void executeGrabCut(float[] foreground, float[] background, int nFgd, int nBgd);
	
	protected void updateFrame() {
		Mat frame = new Mat(getFrameHeight(), getFrameWidth(), CvType.CV_8UC4);
		getMaskedFrame(frame.getNativeObjAddr());
		Bitmap frameBit = Bitmap.createBitmap(frame.cols(), frame.rows(), Bitmap.Config.ARGB_8888);
		Utils.matToBitmap(frame, frameBit);
		setImageBitmap(frameBit);
	}
}
