package edu.ethz.s3d;

import java.util.Vector;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.util.DisplayMetrics;
import android.view.Menu;
import android.view.MenuItem;
import android.view.SubMenu;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.RelativeLayout.LayoutParams;

import com.qualcomm.QCAR.QCAR;

/** The main activity for the ImageTargets sample. */
public class S3D extends Activity {
	// Application status constants:
	private static final int APPSTATUS_UNINITED = -1;
	private static final int APPSTATUS_INIT_APP = 0;
	private static final int APPSTATUS_INIT_QCAR = 1;
	private static final int APPSTATUS_INIT_TRACKER = 2;
	private static final int APPSTATUS_INIT_APP_AR = 3;
	private static final int APPSTATUS_LOAD_TRACKER = 4;
	private static final int APPSTATUS_INITED = 5;
	private static final int APPSTATUS_CAMERA_STOPPED = 6;
	private static final int APPSTATUS_CAMERA_RUNNING = 7;

	// Name of the native dynamic libraries to load:
	private static final String NATIVE_LIB_SAMPLE = "S3D";
	private static final String NATIVE_LIB_QCAR = "QCAR";

	// Our OpenGL view:
	private QCARSampleGLView mGlView;
	private GrabCutView mGrabView;
	
	// Our Reconstruction:
	private Reconstruction mReconstruction = null;

	// The view to display the sample splash screen:
	private ImageView mSplashScreenView;

	// The handler and runnable for the splash screen time out task.
	private Handler mSplashScreenHandler;
	private Runnable mSplashScreenRunnable;

	// The minimum time the splash screen should be visible:
	private static final long MIN_SPLASH_SCREEN_TIME = 2000;

	// The time when the splash screen has become visible:
	long mSplashScreenStartTime = 0;

	// Our renderer:
	private S3DRenderer mRenderer;

	// Display size of the device
	private int mScreenWidth = 0;
	private int mScreenHeight = 0;

	// The current application status
	private int mAppStatus = APPSTATUS_UNINITED;

	// The async tasks to initialize the QCAR SDK
	private InitQCARTask mInitQCARTask;
	private LoadTrackerTask mLoadTrackerTask;

	// An object used for synchronizing QCAR initialization, dataset loading and
	// the Android onDestroy() life cycle event. If the application is destroyed
	// while a data set is still being loaded, then we wait for the loading
	// operation to finish before shutting down QCAR.
	private Object mShutdownLock = new Object();

	// QCAR initialization flags
	private int mQCARFlags = 0;

	// The textures we will use for rendering:
	private Vector<Texture> mTextures;
	private int mSplashScreenImageResource = 0;

	// The menu item for swapping data sets:
	MenuItem mDataSetMenuItem = null;
	
	boolean mIsStonesAndChipsDataSetActive = false;

	/** Static initializer block to load native libraries on start-up. */
	static {
		loadLibrary(NATIVE_LIB_QCAR);
	}

	public void startGrabCutView() {
		RelativeLayout rl = new RelativeLayout(this);
		
		mGlView.onPause();
		mGrabView = new GrabCutView(this, rl);

		// This has to be after the GrabCut Initialization since it needs the width and height.
		if (mReconstruction == null)
			mReconstruction = new Reconstruction(100, 100, 100);

		LayoutParams lpImageView = new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
		rl.addView(mGrabView, lpImageView);
		
		generateGrabViewButtons(rl);
		
		// lpImageView was once new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT));
		// Like this we don't need another object
		addContentView(rl, lpImageView);
		setContentView(rl);
	}
	
	private void generateGrabViewButtons(RelativeLayout rl) {
		// Set LayoutParams
		LayoutParams lpFgd = new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
		LayoutParams lpBgd = new LayoutParams(lpFgd);
		LayoutParams lpDone = new LayoutParams(lpFgd);
		LayoutParams lpReject = new LayoutParams(lpFgd);
		LayoutParams lpMask = new LayoutParams(lpFgd);

		lpFgd.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
		lpFgd.addRule(RelativeLayout.ALIGN_PARENT_LEFT);

		lpBgd.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
		lpBgd.addRule(RelativeLayout.CENTER_HORIZONTAL);

		lpDone.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
		lpDone.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);

		lpReject.addRule(RelativeLayout.ALIGN_PARENT_TOP);
		lpReject.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);

		lpMask.addRule(RelativeLayout.ALIGN_PARENT_TOP);
		lpMask.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
				
		// Create Buttons
		Button doneButton = new Button(this);
		Button rejectButton = new Button(this);
		Button maskButton = new Button(this);
		Button fgButton = new Button(this);
		Button bgButton = new Button(this);
	
		// Set text and action
		doneButton.setText("Accept");
		doneButton.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				finishedGrabCut(true);
			}
		});
		
		rejectButton.setText("Reject");
		rejectButton.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				finishedGrabCut(false);
			}
		});
		
		maskButton.setText("Toggle Mask");
		maskButton.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				mGrabView.switchBitmaps();
			}
		});

		fgButton.setText("select Foreground");
		fgButton.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				mGrabView.isForeground = true;
			}
		});

		bgButton.setText("select Background");
		bgButton.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				mGrabView.isForeground = false;
			}
		});

		// Add Buttons to view
		rl.addView(fgButton, lpFgd);
		rl.addView(bgButton, lpBgd);
		rl.addView(doneButton, lpDone);
		rl.addView(rejectButton, lpReject);
		rl.addView(maskButton, lpMask);

		// Define logic
		fgButton.setEnabled(false);
		bgButton.setEnabled(false);
		doneButton.setEnabled(false);
		maskButton.setEnabled(false);
	}

	public void finishedGrabCut(boolean accept) {
		mGlView.onResume();
		setContentView(mGlView);
		if (accept) {
			mGrabView.moveToStorage();
			mGrabView = null;
			DebugLog.LOGD("Stored Data and resumed Displaying");
			mReconstruction.refineWithLatestCut();
			DebugLog.LOGD("Refined 3D Reconstruction");
		}
	}

	/** An async task to initialize QCAR asynchronously. */
	private class InitQCARTask extends AsyncTask<Void, Integer, Boolean> {
		// Initialize with invalid value
		private int mProgressValue = -1;

		@Override
		protected Boolean doInBackground(Void... params) {
			// Prevent the onDestroy() method to overlap with initialization:
			synchronized (mShutdownLock) {
				QCAR.setInitParameters(S3D.this, mQCARFlags);

				do {
					// QCAR.init() blocks until an initialization step is
					// complete,
					// then it proceeds to the next step and reports progress in
					// percents (0 ... 100%)
					// If QCAR.init() returns -1, it indicates an error.
					// Initialization is done when progress has reached 100%.
					mProgressValue = QCAR.init();

					// Publish the progress value:
					publishProgress(mProgressValue);

					// We check whether the task has been canceled in the
					// meantime
					// (by calling AsyncTask.cancel(true))
					// and bail out if it has, thus stopping this thread.
					// This is necessary as the AsyncTask will run to completion
					// regardless of the status of the component that started
					// is.
				} while (!isCancelled() && mProgressValue >= 0
						&& mProgressValue < 100);

				return (mProgressValue > 0);
			}
		}

		@Override
		protected void onProgressUpdate(Integer... values) {
			// Do something with the progress value "values[0]", e.g. update
			// splash screen, progress bar, etc.
		}

		@Override
		protected void onPostExecute(Boolean result) {
			// Done initializing QCAR, proceed to next application
			// initialization status:
			if (result) {
				DebugLog.LOGD("InitQCARTask::onPostExecute: QCAR initialization"
						+ " successful");

				updateApplicationStatus(APPSTATUS_INIT_TRACKER);
			} else {
				// Create dialog box for display error:
				AlertDialog dialogError = new AlertDialog.Builder(S3D.this)
						.create();
				dialogError.setButton(DialogInterface.BUTTON_POSITIVE, "Close",
						new DialogInterface.OnClickListener() {
							public void onClick(DialogInterface dialog,
									int which) {
								// Exiting application
								System.exit(1);
							}
						});

				String logMessage;

				// NOTE: Check if initialization failed because the device is
				// not supported. At this point the user should be informed
				// with a message.
				if (mProgressValue == QCAR.INIT_DEVICE_NOT_SUPPORTED) {
					logMessage = "Failed to initialize QCAR because this "
							+ "device is not supported.";
				} else {
					logMessage = "Failed to initialize QCAR.";
				}

				// Log error:
				DebugLog.LOGE("InitQCARTask::onPostExecute: " + logMessage
						+ " Exiting.");

				// Show dialog box with error message:
				dialogError.setMessage(logMessage);
				dialogError.show();
			}
		}
	}

	/** An async task to load the tracker data asynchronously. */
	private class LoadTrackerTask extends AsyncTask<Void, Integer, Boolean> {
		@Override
		protected Boolean doInBackground(Void... params) {
			// Prevent the onDestroy() method to overlap:
			synchronized (mShutdownLock) {
				// Load the tracker data set:
				return (loadTrackerData() > 0);
			}
		}

		@Override
		protected void onPostExecute(Boolean result) {
			DebugLog.LOGD("LoadTrackerTask::onPostExecute: execution "
					+ (result ? "successful" : "failed"));

			if (result) {
				// The stones and chips data set is now active:
				mIsStonesAndChipsDataSetActive = true;

				// Done loading the tracker, update application status:
				updateApplicationStatus(APPSTATUS_INITED);
			} else {
				// Create dialog box for display error:
				AlertDialog dialogError = new AlertDialog.Builder(S3D.this)
						.create();
				dialogError.setButton(DialogInterface.BUTTON_POSITIVE, "Close",
						new DialogInterface.OnClickListener() {
							public void onClick(DialogInterface dialog,
									int which) {
								// Exiting application
								System.exit(1);
							}
						});

				// Show dialog box with error message:
				dialogError.setMessage("Failed to load tracker data.");
				dialogError.show();
			}
		}
	}

	private void storeScreenDimensions() {
		// Query display dimensions
		DisplayMetrics metrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(metrics);
		mScreenWidth = metrics.widthPixels;
		mScreenHeight = metrics.heightPixels;
	}

	/**
	 * Called when the activity first starts or the user navigates back to an
	 * activity.
	 */
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		DebugLog.LOGD("ImageTargets::onCreate");
		super.onCreate(savedInstanceState);

		requestWindowFeature(Window.FEATURE_NO_TITLE);

		// Set the splash screen image to display during initialization:
		// TODO: 
		mSplashScreenImageResource = R.drawable.splash_screen;

		DebugLog.LOGI("Trying to load OpenCV library");
		if (!OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_2, this,
				mOpenCVCallBack)) {
			DebugLog.LOGE("Cannot connect to OpenCV Manager");
		}
	}

	private BaseLoaderCallback mOpenCVCallBack = new BaseLoaderCallback(this) {
		@Override
		public void onManagerConnected(int status) {
			DebugLog.LOGD("ImageTargets::BaseLoaderCallback::onManagerConnected");
			switch (status) {
			case LoaderCallbackInterface.SUCCESS: {
				DebugLog.LOGI("OpenCV loaded successfully");

				// Load native library after(!) OpenCV initialization
				loadLibrary(NATIVE_LIB_SAMPLE);

				// Load any sample specific textures:
				mTextures = new Vector<Texture>();
				loadTextures();

				// Query the QCAR initialization flags:
				mQCARFlags = getInitializationFlags();

				// Update the application status to start initializing
				// application
				updateApplicationStatus(APPSTATUS_INIT_APP);
			}
				break;
			default: {
				super.onManagerConnected(status);
			}
				break;
			}
		}
	};

	/**
	 * We want to load specific textures from the APK, which we will later use
	 * for rendering.
	 */
	private void loadTextures() {
		DebugLog.LOGD("ImageTargets::loadTextures");
		mTextures.add(Texture.loadTextureFromApk("TextureTeapotBrass.png",
				getAssets()));
		mTextures.add(Texture.loadTextureFromApk("TextureTeapotBlue.png",
				getAssets()));
		mTextures.add(Texture.loadTextureFromApk("TextureTeapotRed.png",
				getAssets()));
	}

	/** Configure QCAR with the desired version of OpenGL ES. */
	private int getInitializationFlags() {
		DebugLog.LOGD("ImageTargets::getInitializationFlags");
		int flags = 0;

		// Query the native code:
		if (getOpenGlEsVersionNative() == 1) {
			flags = QCAR.GL_11;
		} else {
			flags = QCAR.GL_20;
		}

		return flags;
	}

	/**
	 * native method for querying the OpenGL ES version. Returns 1 for OpenGl ES
	 * 1.1, returns 2 for OpenGl ES 2.0.
	 */
	public native int getOpenGlEsVersionNative();

	/** Native tracker initialization and deinitialization. */
	public native int initTracker();

	public native void deinitTracker();
	
	public native boolean hasTarget();

	/** Native functions to load and destroy tracking data. */
	public native int loadTrackerData();

	public native void destroyTrackerData();

	/** Native sample initialization. */
	public native void onQCARInitializedNative();

	/** Native methods for starting and stoping the camera. */
	private native void startCamera();

	private native void stopCamera();

	/**
	 * Native method for setting / updating the projection matrix for AR content
	 * rendering
	 */
	private native void setProjectionMatrix();

	/** Called when the activity will start interacting with the user. */
	@Override
	protected void onResume() {
		DebugLog.LOGD("ImageTargets::onResume");
		super.onResume();

		// QCAR-specific resume operation
		QCAR.onResume();

		// We may start the camera only if the QCAR SDK has already been
		// initialized
		if (mAppStatus == APPSTATUS_CAMERA_STOPPED) {
			updateApplicationStatus(APPSTATUS_CAMERA_RUNNING);

			// Reactivate flash if it was active before pausing the app
			if (mFlash) {
				boolean result = activateFlash(mFlash);
				DebugLog.LOGI("Turning flash " + (mFlash ? "ON" : "OFF") + " "
						+ (result ? "WORKED" : "FAILED") + "!!");
			}
		}

		// Resume the GL view:
		if (mGlView != null) {
			mGlView.setVisibility(View.VISIBLE);
			mGlView.onResume();
		}
	}

	@Override
	public void onConfigurationChanged(Configuration config) {
		DebugLog.LOGD("ImageTargets::onConfigurationChanged");
		super.onConfigurationChanged(config);

		storeScreenDimensions();

		// Set projection matrix:
		if (QCAR.isInitialized())
			setProjectionMatrix();
	}

	/** Called when the system is about to start resuming a previous activity. */
	@Override
	protected void onPause() {
		DebugLog.LOGD("ImageTargets::onPause");
		super.onPause();

		if (mGlView != null) {
			mGlView.setVisibility(View.INVISIBLE);
			mGlView.onPause();
		}

		if (mAppStatus == APPSTATUS_CAMERA_RUNNING) {
			updateApplicationStatus(APPSTATUS_CAMERA_STOPPED);
		}

		// QCAR-specific pause operation
		QCAR.onPause();
	}

	/** Native function to deinitialize the application. */
	private native void deinitApplicationNative();

	/** The final call you receive before your activity is destroyed. */
	@Override
	protected void onDestroy() {
		DebugLog.LOGD("ImageTargets::onDestroy");
		super.onDestroy();

		// Dismiss the splash screen time out handler:
		if (mSplashScreenHandler != null) {
			mSplashScreenHandler.removeCallbacks(mSplashScreenRunnable);
			mSplashScreenRunnable = null;
			mSplashScreenHandler = null;
		}

		// Cancel potentially running tasks
		if (mInitQCARTask != null
				&& mInitQCARTask.getStatus() != InitQCARTask.Status.FINISHED) {
			mInitQCARTask.cancel(true);
			mInitQCARTask = null;
		}

		if (mLoadTrackerTask != null
				&& mLoadTrackerTask.getStatus() != LoadTrackerTask.Status.FINISHED) {
			mLoadTrackerTask.cancel(true);
			mLoadTrackerTask = null;
		}

		// Ensure that all asynchronous operations to initialize QCAR and
		// loading
		// the tracker datasets do not overlap:
		synchronized (mShutdownLock) {

			// Do application deinitialization in native code
			deinitApplicationNative();

			// Unload texture
			mTextures.clear();
			mTextures = null;

			// Destroy the tracking data set:
			destroyTrackerData();

			// Deinit the tracker:
			deinitTracker();

			// Deinitialize QCAR SDK
			QCAR.deinit();
		}

		System.gc();
	}

	/**
	 * NOTE: this method is synchronized because of a potential concurrent
	 * access by ImageTargets::onResume() and InitQCARTask::onPostExecute().
	 */
	private synchronized void updateApplicationStatus(int appStatus) {
		DebugLog.LOGD("ImageTargets::updateApplicationStatus");
		// Exit if there is no change in status
		if (mAppStatus == appStatus)
			return;

		// Store new status value
		mAppStatus = appStatus;

		// Execute application state-specific actions
		switch (mAppStatus) {
		case APPSTATUS_INIT_APP:
			// Initialize application elements that do not rely on QCAR
			// initialization
			initApplication();

			// Proceed to next application initialization status
			updateApplicationStatus(APPSTATUS_INIT_QCAR);
			break;

		case APPSTATUS_INIT_QCAR:
			// Initialize QCAR SDK asynchronously to avoid blocking the
			// main (UI) thread.
			// This task instance must be created and invoked on the UI
			// thread and it can be executed only once!
			try {
				mInitQCARTask = new InitQCARTask();
				mInitQCARTask.execute();
			} catch (Exception e) {
				DebugLog.LOGE("Initializing QCAR SDK failed");
			}
			break;

		case APPSTATUS_INIT_TRACKER:
			// Initialize the ImageTracker
			if (initTracker() > 0) {
				// Proceed to next application initialization status
				updateApplicationStatus(APPSTATUS_INIT_APP_AR);
			}
			break;

		case APPSTATUS_INIT_APP_AR:
			// Initialize Augmented Reality-specific application elements
			// that may rely on the fact that the QCAR SDK has been
			// already initialized
			initApplicationAR();

			// Proceed to next application initialization status
			updateApplicationStatus(APPSTATUS_LOAD_TRACKER);
			break;

		case APPSTATUS_LOAD_TRACKER:
			// Load the tracking data set
			//
			// This task instance must be created and invoked on the UI
			// thread and it can be executed only once!
			try {
				mLoadTrackerTask = new LoadTrackerTask();
				mLoadTrackerTask.execute();
			} catch (Exception e) {
				DebugLog.LOGE("Loading tracking data set failed");
			}
			break;

		case APPSTATUS_INITED:
			// Hint to the virtual machine that it would be a good time to
			// run the garbage collector.
			//
			// NOTE: This is only a hint. There is no guarantee that the
			// garbage collector will actually be run.
			System.gc();

			// Native post initialization:
			onQCARInitializedNative();

			// The elapsed time since the splash screen was visible:
			long splashScreenTime = System.currentTimeMillis()
					- mSplashScreenStartTime;
			long newSplashScreenTime = 0;
			if (splashScreenTime < MIN_SPLASH_SCREEN_TIME) {
				newSplashScreenTime = MIN_SPLASH_SCREEN_TIME - splashScreenTime;
			}

			// Request a callback function after a given timeout to dismiss
			// the splash screen:
			mSplashScreenHandler = new Handler();
			mSplashScreenRunnable = new Runnable() {
				public void run() {
					// Hide the splash screen
					mSplashScreenView.setVisibility(View.INVISIBLE);

					// Activate the renderer
					mRenderer.mIsActive = true;

					// Now add the GL surface view. It is important
					// that the OpenGL ES surface view gets added
					// BEFORE the camera is started and video
					// background is configured.
					addContentView(mGlView, new LayoutParams(
							LayoutParams.MATCH_PARENT,
							LayoutParams.MATCH_PARENT));
					// addContentView(mGrabView, new
					// LayoutParams(LayoutParams.FILL_PARENT,
					// LayoutParams.FILL_PARENT));

					// Start the camera:
					updateApplicationStatus(APPSTATUS_CAMERA_RUNNING);
				}
			};

			mSplashScreenHandler.postDelayed(mSplashScreenRunnable,
					newSplashScreenTime);
			break;

		case APPSTATUS_CAMERA_STOPPED:
			// Call the native function to stop the camera
			stopCamera();
			break;

		case APPSTATUS_CAMERA_RUNNING:
			// Call the native function to start the camera
			startCamera();
			setProjectionMatrix();
			break;

		default:
			throw new RuntimeException("Invalid application state");
		}
	}

	/** Tells native code whether we are in portait or landscape mode */
	private native void setActivityPortraitMode(boolean isPortrait);

	/** Initialize application GUI elements that are not related to AR. */
	private void initApplication() {
		// Set the screen orientation
		//
		// NOTE: It is recommended to set this because of the following reasons:
		//
		// 1.) Before Android 2.2 there is no reliable way to query the
		// absolute screen orientation from an activity, therefore using
		// an undefined orientation is not recommended. Screen
		// orientation matching orientation sensor measurements is also
		// not recommended as every screen orientation change triggers
		// deinitialization / (re)initialization steps in internal QCAR
		// SDK components resulting in unnecessary overhead during
		// application run-time.
		//
		// 2.) Android camera drivers seem to always deliver landscape images
		// thus QCAR SDK components (e.g. camera capturing) need to know
		// when we are in portrait mode. Before Android 2.2 there is no
		// standard, device-independent way to let the camera driver know
		// that we are in portrait mode as each device seems to require a
		// different combination of settings to rotate camera preview
		// frames images to match portrait mode views. Because of this,
		// we suggest that the activity using the QCAR SDK be locked
		// to landscape mode if you plan to support Android 2.1 devices
		// as well. Froyo is fine with both orientations.
		int screenOrientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE;

		// Apply screen orientation
		setRequestedOrientation(screenOrientation);

		// Pass on screen orientation info to native code
		setActivityPortraitMode(screenOrientation == ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);

		storeScreenDimensions();

		// As long as this window is visible to the user, keep the device's
		// screen turned on and bright.
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
				WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		// Create and add the splash screen view
		mSplashScreenView = new ImageView(this);
		mSplashScreenView.setImageResource(mSplashScreenImageResource);
		addContentView(mSplashScreenView, new LayoutParams(
				LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT));

		mSplashScreenStartTime = System.currentTimeMillis();

	}

	/** Native function to initialize the application. */
	private native void initApplicationNative(int width, int height);

	/** Initializes AR application components. */
	private void initApplicationAR() {
		// Do application initialization in native code (e.g. registering
		// callbacks, etc.)
		initApplicationNative(mScreenWidth, mScreenHeight);

		// Create OpenGL ES view:
		int depthSize = 16;
		int stencilSize = 0;
		boolean translucent = QCAR.requiresAlpha();

		mGlView = new QCARSampleGLView(this);
		mGlView.init(mQCARFlags, translucent, depthSize, stencilSize);

		mRenderer = new S3DRenderer();
		mGlView.setRenderer(mRenderer);

	}

	/**
	 * Invoked the first time when the options menu is displayed to give the
	 * Activity a chance to populate its Menu with menu items.
	 */
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		super.onCreateOptionsMenu(menu);

		mDataSetMenuItem = menu.add("Switch to Tarmac dataset");
		menu.add("Toggle flash");
		menu.add("Trigger autofocus");

		SubMenu focusModes = menu.addSubMenu("Focus Modes");
		focusModes.add("Normal").setCheckable(true);
		focusModes.add("Continuous Autofocus").setCheckable(true);
		focusModes.add("Infinity").setCheckable(true);
		focusModes.add("Macro Mode").setCheckable(true);

		return true;
	}

	/** Tells native code to switch dataset as soon as possible */
	private native void switchDatasetAsap();

	/** Invoked when the user selects an item from the Menu */
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		if (item == mDataSetMenuItem) {
			switchDatasetAsap();
			mIsStonesAndChipsDataSetActive = !mIsStonesAndChipsDataSetActive;
			if (mIsStonesAndChipsDataSetActive) {
				item.setTitle("Switch to Tarmac dataset");
			} else {
				item.setTitle("Switch to StonesAndChips dataset");
			}

		} else if (item.getTitle().equals("Toggle flash")) {
			mFlash = !mFlash;
			boolean result = activateFlash(mFlash);
			DebugLog.LOGI("Turning flash " + (mFlash ? "ON" : "OFF") + " "
					+ (result ? "WORKED" : "FAILED") + "!!");
		} else if (item.getTitle().equals("Trigger autofocus")) {
			boolean result = autofocus();
			DebugLog.LOGI("Autofocus requested"
					+ (result ? " successfully."
							: ".  Not supported in current mode or on this device."));
		} else {
			int arg = -1;
			if (item.getTitle().equals("Normal"))
				arg = 0;
			if (item.getTitle().equals("Continuous Autofocus"))
				arg = 1;
			if (item.getTitle().equals("Infinity"))
				arg = 2;
			if (item.getTitle().equals("Macro Mode"))
				arg = 3;

			if (arg != -1) {
				boolean result = setFocusMode(arg);
				if (result) {
					item.setChecked(true);
					if (checked != null && item != checked)
						checked.setChecked(false);
					checked = item;
				}

				DebugLog.LOGI("Requested Focus mode "
						+ item.getTitle()
						+ (result ? " successfully."
								: ".  Not supported on this device."));
			}
		}

		return true;
	}

	private MenuItem checked;
	private boolean mFlash = false;

	private native boolean activateFlash(boolean flash);

	private native boolean autofocus();

	private native boolean setFocusMode(int mode);

	/** Returns the number of registered textures. */
	public int getTextureCount() {
		return mTextures.size();
	}

	/** Returns the texture object at the specified index. */
	public Texture getTexture(int i) {
		return mTextures.elementAt(i);
	}

	/** A helper for loading native libraries stored in "libs/armeabi*". */
	public static boolean loadLibrary(String nLibName) {
		try {
			System.loadLibrary(nLibName);
			DebugLog.LOGI("Native library lib" + nLibName + ".so loaded");
			return true;
		} catch (UnsatisfiedLinkError ulee) {
			DebugLog.LOGE("The library lib" + nLibName
					+ ".so could not be loaded");
		} catch (SecurityException se) {
			DebugLog.LOGE("The library lib" + nLibName
					+ ".so was not allowed to be loaded");
		}

		return false;
	}
}
