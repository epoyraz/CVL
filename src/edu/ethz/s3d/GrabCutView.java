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
import android.os.AsyncTask;
import android.util.FloatMath;
import android.view.MotionEvent;
import android.view.MotionEvent.PointerCoords;
import android.view.View;
import android.view.View.OnTouchListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.RelativeLayout;

public class GrabCutView extends ImageView implements OnTouchListener {
	private Object mShutdownLock = new Object();
	
	private Rect initRect;
	private int frameWidth;
	private int frameHeight;
	
	private RelativeLayout parentLayout;
	private RectGrabCutTask mRectTask;
	private StrokeGrabCutTask mStrokeTask;
	
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
        setOnTouchListener(this);
        
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
	
	public GrabCutView(Context context, RelativeLayout layoutView) {
		super(context);
		
		parentLayout = layoutView;
				
		//scale on RelativeLayout
		setAdjustViewBounds(true);
		setScaleType(ScaleType.CENTER_CROP);
        setOnTouchListener(this);
        
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
		//DebugLog.LOGD("S3DView::onTouch");
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
					disableInput();
					try {
						mStrokeTask = new StrokeGrabCutTask();
						mStrokeTask.execute();
					} catch (Exception e) {
						DebugLog.LOGE("Executing Stroke Task failed");
					}
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
					disableInput();
					try {
						mRectTask = new RectGrabCutTask();
						mRectTask.execute();
					} catch (Exception e) {
						DebugLog.LOGE("Executing Rect Task failed");
					}
					break;
						
			}
		}
		invalidate();
		// We want to get all follow-up events.
		return true;
	}

    private float[] convertToArray(LinkedList<LinkedList<PointerCoords>> strokes) {
    	//Initialize the lists
    	ArrayList<Float> list = new ArrayList<Float>();
    	Iterator<LinkedList<MotionEvent.PointerCoords>> outerIter = strokes.iterator();
    	
    	// Loop through the strokes
    	while (outerIter.hasNext()) {
    		Iterator<MotionEvent.PointerCoords> innerIter = outerIter.next().iterator();
    		// Initialize the last element and add it to the output points
    		MotionEvent.PointerCoords last = null;
    		if (innerIter.hasNext()) {
    			last = innerIter.next();
    			list.add(last.x / scale + wSupplement);
    			list.add(last.y / scale + hSupplement);
    		}
    		// Loop through the inner list
    		while (innerIter.hasNext()) {
    			MotionEvent.PointerCoords coord = innerIter.next();
    			float distX = coord.x - last.x;
    			float distY = coord.y - last.y;
    			int dist = (int) FloatMath.floor(FloatMath.sqrt(distX*distX + distY*distY)/scale);
    			distX /= dist;
    			distY /= dist;
    			// Insert all points on the line
    			for (int i = 1; i <= dist; i++) {
	    			list.add((float)Math.round((last.x + distX*i) / scale + wSupplement));
	    			list.add((float)Math.round((last.y + distY*i) / scale + hSupplement));
    			}
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
        //DebugLog.LOGD("S3DView::onDraw");
        
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
	public native void moveToStorage();
	
	protected void updateFrame() {
		frameHeight = getFrameHeight();
		frameWidth = getFrameWidth();
		Mat frame = new Mat(frameHeight, frameWidth, CvType.CV_8UC4);
		getMaskedFrame(frame.getNativeObjAddr());
		Bitmap frameBit = Bitmap.createBitmap(frame.cols(), frame.rows(), Bitmap.Config.ARGB_8888);
		Utils.matToBitmap(frame, frameBit);
		setImageBitmap(frameBit);
	}
	
	private void disableInput() {
		setOnTouchListener(null);
		setColorFilter(Color.GRAY, android.graphics.PorterDuff.Mode.LIGHTEN);
		Button button = (Button) parentLayout.getChildAt(1);
		button.setEnabled(false);
		button = (Button) parentLayout.getChildAt(2);
		button.setEnabled(false);
		button = (Button) parentLayout.getChildAt(3);
		button.setEnabled(false);
	}
	
	private void enableInput() {
        setOnTouchListener(this);
		setColorFilter(Color.GRAY, android.graphics.PorterDuff.Mode.DST);
		Button button = (Button) parentLayout.getChildAt(1);
		button.setEnabled(true);
		button = (Button) parentLayout.getChildAt(2);
		button.setEnabled(true);
		button = (Button) parentLayout.getChildAt(3);
		button.setEnabled(true);
	}

	/** An async task to calculate the GrabCut using some Strokes. */
	private class RectGrabCutTask extends AsyncTask<Void, Integer, Boolean> {
		@Override
		protected Boolean doInBackground(Void... params) {
			// Prevent the onDestroy() method to overlap with initialization:
			synchronized (mShutdownLock) {
				try {
					calculateScale();
					DebugLog.LOGI("Left "+initRect.left+", Top "+initRect.top+", Right "+initRect.right+", Bottom "+ initRect.bottom);
					int left = (int)(initRect.left / scale + wSupplement);
					int top = (int)(initRect.top / scale + hSupplement);
					int right = (int)(initRect.right / scale + wSupplement);
					int bottom = (int)(initRect.bottom / scale + hSupplement);
					DebugLog.LOGI("Left "+left+", Top "+top+", Right "+right+", Bottom "+ bottom);
					DebugLog.LOGI("wSupp: "+wSupplement+" hSupp: "+hSupplement);
					initGrabCut(left, top, right, bottom);
					hasRect = true;
				}
				catch (Exception e) {
					DebugLog.LOGE(e.getMessage());
				}
			}
			return true;
		}
		@Override
		protected void onPostExecute(Boolean result) {
			try {
				enableInput();
				updateFrame();
			}
			catch (Exception e) {
				DebugLog.LOGE(e.getMessage());
			}
			DebugLog.LOGD("Finished RectTask");
		}
	}
	
	/** An async task to calculate the GrabCut using some Strokes. */
	private class StrokeGrabCutTask extends AsyncTask<Void, Integer, Boolean> {
		@Override
		protected Boolean doInBackground(Void... params) {
			// Prevent the onDestroy() method to overlap with initialization:
			synchronized (mShutdownLock) {
				try {
					float[] fgdCoords = convertToArray(fgdStrokes);
					float[] bgdCoords = convertToArray(bgdStrokes);
					executeGrabCut(fgdCoords, bgdCoords, fgdCoords.length/2, bgdCoords.length/2);
				}
				catch (Exception e) {
					DebugLog.LOGE(e.getMessage());
				}
			}
			return true;
		}
		@Override
		protected void onPostExecute(Boolean result) {
			try {
				enableInput();
				updateFrame();
			}
			catch (Exception e) {
				DebugLog.LOGE(e.getMessage());
			}
			DebugLog.LOGD("Finished StrokeTask");
		}
	}
}
