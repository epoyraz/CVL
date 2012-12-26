/*==============================================================================
            Copyright (c) 2012 QUALCOMM Austria Research Center GmbH.
            All Rights Reserved.
            Qualcomm Confidential and Proprietary
            
@file 
    ImageTargetsRenderer.java

@brief
    Sample for ImageTargets

==============================================================================*/


package edu.ethz.s3d;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.opengl.GLSurfaceView;

import com.qualcomm.QCAR.QCAR;


/** The renderer class for the ImageTargets sample. */
public class S3DRenderer implements GLSurfaceView.Renderer
{
    public boolean mIsActive = false;
    
    /** Native function for initializing the renderer. */
    public native void initRendering();
    
    
    /**
     * Native function to update the renderer
     * @param width The width of the screen
     * @param height The height of the screen
     */
    public native void updateRendering(int width, int height);

    /**
     * Initializes the rendering and relays the information to vuforia
     * @param gl The OpenGL environment binding
     * @param config The OpenGL Configuration
     */
    public void onSurfaceCreated(GL10 gl, EGLConfig config)
    {
        DebugLog.LOGD("GLRenderer::onSurfaceCreated");

        // Call native function to initialize rendering:
        initRendering();
        
        // Call QCAR function to (re)initialize rendering after first use
        // or after OpenGL ES context was lost (e.g. after onPause/onResume):
        QCAR.onSurfaceCreated();
    }
    
    
    /**
     * Called when the surface changed size.
     * @param gl The OpenGL environment
     * @param width The new width of the screen
     * @param height The new height of the screen
     */
    public void onSurfaceChanged(GL10 gl, int width, int height)
    {
        DebugLog.LOGD("GLRenderer::onSurfaceChanged");
        
        // Call native function to update rendering when render surface parameters have changed:
        updateRendering(width, height);

        // Call QCAR function to handle render surface size changes:
        QCAR.onSurfaceChanged(width, height);
    }    
    
    
    /** The native render function. */    
    public native void renderFrame();
    
    
    /**
     * Called to draw the current frame.
     * @param gl The OpenGL environment
     */
    public void onDrawFrame(GL10 gl)
    {
        if (!mIsActive)
            return;

        // Call our native function to render content
        renderFrame();
    }
}
