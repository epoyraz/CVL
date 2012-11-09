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
import android.graphics.Rect;
import android.graphics.Paint.Style;
import android.view.MotionEvent;
import android.view.MotionEvent.PointerCoords;
import android.view.View;
import android.view.View.OnTouchListener;
import android.widget.ImageView;

public class GrabCutView extends ImageView implements OnTouchListener {
	private Rect initRect;
	
	private int frameWidth;
	private int frameHeight;
	
	private LinkedList<LinkedList<MotionEvent.PointerCoords>> fgdStrokes;
	private LinkedList<LinkedList<MotionEvent.PointerCoords>> bgdStrokes;
	public boolean isForeground = true;
	private boolean hasRect = false;
	private boolean drawRect = false;
	private Paint initRectColor = new Paint();
	private Paint fgdColor = new Paint();
	private Paint bgdColor = new Paint();
	
	float scale;
	int hSupplement;
	int wSupplement;

	public GrabCutView(Context context) {
		super(context);
		
		//scale on RelativeLayout
		setAdjustViewBounds(true);
		setScaleType(ScaleType.CENTER_CROP);
		// Set everything so the drawing will work
        setOnTouchListener(this);
        //this.setWillNotDraw(false);
        
        // Initialize drawing stuff
        fgdStrokes = new LinkedList<LinkedList<MotionEvent.PointerCoords>>();
        bgdStrokes = new LinkedList<LinkedList<MotionEvent.PointerCoords>>();
        fgdColor.setColor(Color.BLUE);
        fgdColor.setStrokeWidth(5);
        bgdColor.setColor(Color.GREEN);
        bgdColor.setStrokeWidth(5);
        initRectColor.setColor(Color.RED);
        initRectColor.setStrokeWidth(5);
        initRectColor.setStyle(Style.STROKE);
        
        // Initialize Image
        initRect = new Rect();
		grabFrame();
		setScaleType(ScaleType.CENTER_CROP);
        updateFrame();
	}
	
	private void calculateScale() {
        int width = getWidth();
    	int height = getHeight();
    	float wScale = width / frameWidth;
    	float hScale = height / frameHeight;
    	scale = Math.max(wScale, hScale);
    	double wOverhang = frameWidth - (width / scale);
    	double hOverhang = frameHeight - (height / scale);
    	wSupplement = (int) Math.floor(wOverhang/2);
    	hSupplement = (int) Math.floor(hOverhang/2);
	}
	
    public boolean onTouch(View v, MotionEvent event) {
		DebugLog.LOGD("S3DView::onTouch");
		// Select corresponding list
		LinkedList<LinkedList<MotionEvent.PointerCoords>> outerList = isForeground ? fgdStrokes : bgdStrokes;
		// Initialize coordinates object
		MotionEvent.PointerCoords coordinates = new MotionEvent.PointerCoords();
		
		if (hasRect) {
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
					executeGrabCut(fgdCoords, bgdCoords, fgdCoords.length/2, bgdCoords.length/2);
					updateFrame();
					break;
			}
		}
		else {
			// Switch the different event types
			switch (event.getAction()) {
				case MotionEvent.ACTION_DOWN:
					// We store the first position of the touch by generating a new list
					event.getPointerCoords(0, coordinates);
					initRect.set((int)coordinates.x, (int)coordinates.y, (int)coordinates.x, (int)coordinates.y);
					drawRect = true;
					break;
				case MotionEvent.ACTION_MOVE:
					// We get the latest position information
					event.getPointerCoords(0, coordinates);
					initRect.union((int)coordinates.x, (int)coordinates.y);
					break;
				case MotionEvent.ACTION_UP:
					calculateScale();
					DebugLog.LOGI("Left "+initRect.left+", Top "+initRect.top+", Right "+initRect.right+", Bottom "+ initRect.bottom);
					int left = (int)(initRect.left / scale + wSupplement);
					int top = (int)(initRect.top / scale + hSupplement);
					int right = (int)(initRect.right / scale + wSupplement);
					int bottom = (int)(initRect.bottom / scale + hSupplement);
					DebugLog.LOGI("Left "+left+", Top "+top+", Right "+right+", Bottom "+ bottom);
					DebugLog.LOGI("wSupp: "+wSupplement+" hSupp: "+hSupplement);
					initGrabCut(left, top, right, bottom);
					updateFrame();
					hasRect = true;
					break;
						
			}
		}
		invalidate();
		// We want to get all follow-up events.
		return true;
	}

    private float[] convertToArray(LinkedList<LinkedList<PointerCoords>> strokes) {
    	
    	ArrayList<Float> list = new ArrayList<Float>();
    	Iterator<LinkedList<MotionEvent.PointerCoords>> outerIter = strokes.iterator();
    	while (outerIter.hasNext()) {
    		Iterator<MotionEvent.PointerCoords> innerIter = outerIter.next().iterator();
    		while (innerIter.hasNext()) {
    			MotionEvent.PointerCoords coord = innerIter.next();
    			list.add(coord.x / scale + wSupplement);
    			list.add(coord.y / scale + hSupplement);
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
        
        if (drawRect) {
        	canvas.drawRect(initRect, initRectColor);
        }
    	
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
	protected native void initGrabCut(int left, int top, int right, int bottom);
	protected native void executeGrabCut(float[] foreground, float[] background, int nFgd, int nBgd);
	
	protected void updateFrame() {
		frameHeight = getFrameHeight();
		frameWidth = getFrameWidth();
		Mat frame = new Mat(frameHeight, frameWidth, CvType.CV_8UC4);
		getMaskedFrame(frame.getNativeObjAddr());
		Bitmap frameBit = Bitmap.createBitmap(frame.cols(), frame.rows(), Bitmap.Config.ARGB_8888);
		Utils.matToBitmap(frame, frameBit);
		setImageBitmap(frameBit);
	}
}
