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

/**
 * Is a View which handles all GrabCut parts 
 * @author gostefan
 */
public class GrabCutView extends ImageView implements OnTouchListener {
	/**
	 * A lock to prevent a null pointer exception in case of a manual shutdown 
	 */
	private Object mShutdownLock = new Object();
	/**
	 * Tells in which phase we are
	 * False means phase 1 (rectangle selection)
	 * True means phase 2 (strokes improvement)
	 */
	private boolean hasRect = false;
	
	/**
	 * The current rectangle used for initialization
	 */
	private Rect initRect;
	/**
	 * The async task object for  rectangle initialization
	 */
	private RectGrabCutTask mRectTask;
	/**
	 * Tells whether to draw the rectangle or not
	 */
	private boolean drawRect = false;
	/**
	 * The color of the rectangle
	 */
	private Paint initRectColor = new Paint();
	
	/**
	 * The frame width returned from the native code
	 */
	private int frameWidth;
	/**
	 * The frame height returned from the native code
	 */
	private int frameHeight;
	
	/**
	 * The relative layout which holds the input buttons
	 */
	private RelativeLayout parentLayout;
	
	/**
	 * The async task object for stroke improvement
	 */
	private StrokeGrabCutTask mStrokeTask;
	/**
	 * All foreground strokes
	 */
	private LinkedList<LinkedList<MotionEvent.PointerCoords>> fgdStrokes;
	/**
	 * All background strokes
	 */
	private LinkedList<LinkedList<MotionEvent.PointerCoords>> bgdStrokes;
	/**
	 * Tells whether we're currently drawing a foreground or background stroke
	 */
	public boolean isForeground = true;
	/**
	 * The color of foreground strokes
	 */
	private Paint fgdColor = new Paint();
	/**
	 * The color of background strokes
	 */
	private Paint bgdColor = new Paint();
		
	/**
	 * The bitmap with the masked frame
	 */
	private Bitmap frameBit;
	/**
	 * The bitmap with the bw-mask
	 */
	private Bitmap maskBit;
	/**
	 * Tells whether to show the frame or the bw-mask
	 */
	boolean showFrame = true;
	
	float scale;
	int hSupplement;
	int wSupplement;

	/**
	 * This Constructor is primarely to avoid warnings. If you initialize with this constructor, the view will probably not work.
	 * @param context The parent activity
	 */
	public GrabCutView(Context context) {
		//TODO: Find a better solution for this 
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
	
	/**
	 * This constructor initializes all its private fields and gets the current frame to compute the GrabCut on
	 * @param context The parent activity
	 * @param layoutView The parent layout view (with the buttons on it)
	 */
	public GrabCutView(Context context, RelativeLayout layoutView) {
		//TODO: We don't really know that the buttons are in the parent unless we put them there ourselves
		super(context);
		
		parentLayout = layoutView;
				
		// scale on RelativeLayout
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
	
	/**
	 * Calculates all the figures and scales (images and texture)
	 */
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
	
	/**
	 * Handles the touches
	 * In the first part it gets the position and creates a rectangle, in the second part it generates multiple strokes
	 * @param v The view on which the touch happened
	 * @param event All the data corresponding to the current touch
	 * @return Always returns true since we want to get all follow-up events and we consumed the event
	 */
    public boolean onTouch(View v, MotionEvent event) {
    	// TODO: Refactor
		// Select corresponding list
		LinkedList<LinkedList<MotionEvent.PointerCoords>> outerList = isForeground ? fgdStrokes : bgdStrokes;
		// Initialize coordinates object
		MotionEvent.PointerCoords coordinates = new MotionEvent.PointerCoords();
		
		// Distinguish in which phase we are
		if (hasRect) {
			// 2nd phase
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
			// 1st phase
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
		
		// We invalidate the view since we want to update the screen
		invalidate();
		// We want to get all follow-up events.
		return true;
	}

    /**
     * Converts the internal storage format of strokes to points which can be passed to the native implementation
     * @param strokes A strokes list
     * @return An array with all points created from the strokes list
     */
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
    		// Loop through the point list
    		while (innerIter.hasNext()) {
    			// Calculate direction and length of current leg
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
    			last = coord;
    		}
    	}
    	
    	// Convert the list of floats to an array of floats
    	float[] finalArray = new float[list.size()];
    	Iterator<Float> iter = list.iterator();
    	int i = 0;
    	while (iter.hasNext()) {
    		finalArray[i] = iter.next();
    		i++;
    	}
		return finalArray;
	}

    /**
     * Draws the rectangle and strokes on the screen
     * @param canvas The canvas on which to draw the rectangle and strokes
     */
	@Override
    public void onDraw(Canvas canvas) {
		super.onDraw(canvas);
        
		// Draw the rectangle
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
	
	/**
	 * Gets the current frame overlaid with the fore-/background mask
	 * @param address The native address of the opencv matrix in which we want to load the data
	 */
	protected native void getMaskedFrame(long address);
	/**
	 * Gets the fore-/background mask
	 * @param address The native address of the opencv matrix in which we want to load the data
	 */
	protected native void getMask(long address);
	/**
	 * Gets the height of the frames
	 * @return The height of the frames
	 */
	protected native int getFrameHeight();
	/**
	 * Gets the width of the frames
	 * @return The width of the frames
	 */
	protected native int getFrameWidth();
	/**
	 * Tells the native code to get the newest frame from the camera and store it
	 */
	protected native void grabFrame();
	/**
	 * Initializes the GrabCut with the current rectangle coordinates
	 * @param left The left distance of the rectangle
	 * @param top The top distance of the rectangle
	 * @param right The right distance of the rectangle (from the left border)
	 * @param bottom The bottom distance of the rectangle (from the top border)
	 */
	protected native void initGrabCut(int left, int top, int right, int bottom);
	/**
	 * This actually executes the GrabCut
	 * @param foreground A float array with all foreground points
	 * @param background A float array with all background points
	 * @param nFgd How many foreground points are in the array
	 * @param nBgd How many background points are in the array
	 */
	protected native void executeGrabCut(float[] foreground, float[] background, int nFgd, int nBgd);
	/**
	 * Tells the native code to move the current mask and projection matrix into the silhouette storage
	 */
	public native void moveToStorage();
	
	/**
	 * Gets the current frame from the native code and displays it
	 */
	protected void updateFrame() {
		frameHeight = getFrameHeight();
		frameWidth = getFrameWidth();
		Mat frame = new Mat(frameHeight, frameWidth, CvType.CV_8UC4);
		getMaskedFrame(frame.getNativeObjAddr());
		frameBit = Bitmap.createBitmap(frame.cols(), frame.rows(), Bitmap.Config.ARGB_8888);
		Utils.matToBitmap(frame, frameBit);
		setImageBitmap(frameBit);
	}
	
	/**
	 * Disables all input so the computations can work quietly
	 */
	private void disableInput() {
		setOnTouchListener(null);
		setColorFilter(Color.GRAY, android.graphics.PorterDuff.Mode.LIGHTEN);
		Button button = (Button) parentLayout.getChildAt(1);
		button.setEnabled(false);
		button = (Button) parentLayout.getChildAt(2);
		button.setEnabled(false);
		button = (Button) parentLayout.getChildAt(3);
		button.setEnabled(false);
		button = (Button) parentLayout.getChildAt(4);
		button.setEnabled(false);
		button = (Button) parentLayout.getChildAt(5);
		button.setEnabled(false);
	}
	
	/**
	 * Enables input after the computations are done and gets the current mask
	 */
	private void enableInput() {
        setOnTouchListener(this);
		setColorFilter(Color.GRAY, android.graphics.PorterDuff.Mode.DST);
		Button button = (Button) parentLayout.getChildAt(1);
		button.setEnabled(true);
		button = (Button) parentLayout.getChildAt(2);
		button.setEnabled(true);
		button = (Button) parentLayout.getChildAt(3);
		button.setEnabled(true);
		button = (Button) parentLayout.getChildAt(4);
		button.setEnabled(true);
		button = (Button) parentLayout.getChildAt(5);
		button.setEnabled(true);
		
		// TODO: Move this into updateFrame
		Mat mask = new Mat(frameHeight, frameWidth, CvType.CV_8UC4);
		getMask(mask.getNativeObjAddr());
		maskBit = Bitmap.createBitmap(frameBit.getWidth(), frameBit.getHeight(), Bitmap.Config.ARGB_8888);
		Utils.matToBitmap(mask, maskBit);
	}

	/**
	 * Switches between the masked frame and the bw-mask
	 */
	public void switchBitmaps() {
		setImageBitmap(showFrame ? maskBit : frameBit);
		showFrame = !showFrame;
	}

	/**
	 *  An async task to calculate the GrabCut using some Strokes.
	 */
	private class RectGrabCutTask extends AsyncTask<Void, Integer, Boolean> {
		/**
		 * This executes the GrabCut initialization using the specified rectangle in a new thread
		 * @return Returns always true
		 */
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
		
		/**
		 * Enables the input and updates the frame data
		 */
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
	
	/** 
	 * An async task to calculate the GrabCut using some Strokes.
	 */
	private class StrokeGrabCutTask extends AsyncTask<Void, Integer, Boolean> {
		/**
		 * This executes the GrabCut improvement using the specified strokes in a new thread
		 * @return Returns always true
		 */
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
		/**
		 * Enables the input and updates the frame data
		 */
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
