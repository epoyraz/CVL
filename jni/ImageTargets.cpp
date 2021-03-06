/*==============================================================================
            Copyright (c) 2012 QUALCOMM Austria Research Center GmbH.
            All Rights Reserved.
            Qualcomm Confidential and Proprietary
            
@file 
    ImageTargets.cpp

@brief
    Sample for ImageTargets

==============================================================================*/


#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef USE_OPENGL_ES_1_1
#include <GLES/gl.h>
#include <GLES/glext.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

#include <QCAR/QCAR.h>
#include <QCAR/CameraDevice.h>
#include <QCAR/Renderer.h>
#include <QCAR/VideoBackgroundConfig.h>
#include <QCAR/Trackable.h>
#include <QCAR/Tool.h>
#include <QCAR/Tracker.h>
#include <QCAR/TrackerManager.h>
#include <QCAR/ImageTracker.h>
#include <QCAR/CameraCalibration.h>
#include <QCAR/UpdateCallback.h>
#include <QCAR/DataSet.h>
#include "SampleMath.h"
#include "S3D.h"
#include "GrabCut.h"
#include "SampleUtils.h"
#include "Texture.h"
#include "CubeShaders.h"
#include "Teapot.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Textures:
int textureCount                = 0;
Texture** textures              = 0;
unsigned int texId = 0;
unsigned int backgroundTexture  = 0;

// OpenGL ES 2.0 specific:
#define USE_OPENGL_ES_2_0
#ifdef USE_OPENGL_ES_2_0
unsigned int shaderProgramID    = 0;
GLint vertexHandle              = 0;
GLint normalHandle              = 0;
GLint mvpMatrixHandle           = 0;
GLint vertexColorHandle			= 0;
#endif

// Screen dimensions:
unsigned int screenWidth        = 0;
unsigned int screenHeight       = 0;

// Indicates whether screen is in portrait (true) or landscape (false) mode
bool isActivityInPortraitMode   = false;

// The projection matrix used for rendering virtual objects:
QCAR::Matrix44F projectionMatrix;

// Constants:
// Attention! The same value is also defined in GrabCut.cpp
static const float kObjectScale = 3.f;

QCAR::DataSet* dataSetStonesAndChips    = 0;
QCAR::DataSet* dataSetTarmac            = 0;

bool switchDataSetAsap          = false;

// Switches the dataset
class ImageTargets_UpdateCallback : public QCAR::UpdateCallback
{
    virtual void QCAR_onUpdate(QCAR::State& /*state*/)
    {
        if (switchDataSetAsap)
        {
            switchDataSetAsap = false;

            // Get the image tracker:
            QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
            QCAR::ImageTracker* imageTracker = static_cast<QCAR::ImageTracker*>(trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER));
            if (imageTracker == 0 || dataSetStonesAndChips == 0 || dataSetTarmac == 0 || imageTracker->getActiveDataSet() == 0) {
                LOG("Failed to switch data set.");
                return;
            }
            
            if (imageTracker->getActiveDataSet() == dataSetStonesAndChips) {
                imageTracker->deactivateDataSet(dataSetStonesAndChips);
                imageTracker->activateDataSet(dataSetTarmac);
            }
            else {
                imageTracker->deactivateDataSet(dataSetTarmac);
                imageTracker->activateDataSet(dataSetStonesAndChips);
            }
        }
    }
};

ImageTargets_UpdateCallback updateCallback;

// Just returns 1 for OpenGLEs 1.1 and 2 for other
JNIEXPORT int JNICALL
Java_edu_ethz_s3d_S3D_getOpenGlEsVersionNative(JNIEnv *, jobject)
{
#ifdef USE_OPENGL_ES_1_1        
    return 1;
#else
    return 2;
#endif
}

// Just returns 1 for OpenGLEs 1.1 and 2 for other
JNIEXPORT bool JNICALL
Java_edu_ethz_s3d_S3D_hasTarget(JNIEnv *, jobject)
{
	// Get the state from QCAR and mark the beginning of a rendering section
    QCAR::State state = QCAR::Renderer::getInstance().begin();

    return (state.getNumActiveTrackables() > 0);
}


// Sets isActivityInPortraitMode to the argument value
JNIEXPORT void JNICALL
Java_edu_ethz_s3d_S3D_setActivityPortraitMode(JNIEnv *, jobject, jboolean isPortrait)
{
    isActivityInPortraitMode = isPortrait;
}

// Sets the boolean to switch the dataset
JNIEXPORT void JNICALL
Java_edu_ethz_s3d_S3D_switchDatasetAsap(JNIEnv *, jobject)
{
    switchDataSetAsap = true;
}

// Tries to initialize the Image tracker (returns 1 on success, 0 on fail)
JNIEXPORT int JNICALL
Java_edu_ethz_s3d_S3D_initTracker(JNIEnv *, jobject)
{
    LOG("Java_edu_ethz_s3d_S3D_initTracker");
    
    // Initialize the image tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::Tracker* tracker = trackerManager.initTracker(QCAR::Tracker::IMAGE_TRACKER);
    if (tracker == NULL) {
        LOG("Failed to initialize ImageTracker.");
        return 0;
    }

    LOG("Successfully initialized ImageTracker.");
    return 1;
}

// Deinitializes the tracker
JNIEXPORT void JNICALL
Java_edu_ethz_s3d_S3D_deinitTracker(JNIEnv *, jobject)
{
    LOG("Java_edu_ethz_s3d_S3D_deinitTracker");

    // Deinit the image tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    trackerManager.deinitTracker(QCAR::Tracker::IMAGE_TRACKER);
}

// Loads the tracker image data and activates the StonesAndChips dataset (returns 1 on success, 0 on fail)
JNIEXPORT int JNICALL
Java_edu_ethz_s3d_S3D_loadTrackerData(JNIEnv *, jobject)
{
    LOG("Java_edu_ethz_s3d_S3D_loadTrackerData");
    
    // Get the image tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::ImageTracker* imageTracker = static_cast<QCAR::ImageTracker*>(trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER));
    if (imageTracker == NULL) {
        LOG("Failed to load tracking data set because the ImageTracker has not been initialized.");
        return 0;
    }

    // Create the data sets:
    dataSetStonesAndChips = imageTracker->createDataSet();
    if (dataSetStonesAndChips == 0) {
        LOG("Failed to create a new tracking data.");
        return 0;
    }

    dataSetTarmac = imageTracker->createDataSet();
    if (dataSetTarmac == 0) {
        LOG("Failed to create a new tracking data.");
        return 0;
    }

    // Load the data sets:
    if (!dataSetStonesAndChips->load("StonesAndChips.xml", QCAR::DataSet::STORAGE_APPRESOURCE)) {
        LOG("Failed to load data set.");
        return 0;
    }

    if (!dataSetTarmac->load("Tarmac.xml", QCAR::DataSet::STORAGE_APPRESOURCE)) {
        LOG("Failed to load data set.");
        return 0;
    }

    // Activate the data set:
    if (!imageTracker->activateDataSet(dataSetStonesAndChips)) {
        LOG("Failed to activate data set.");
        return 0;
    }

    LOG("Successfully loaded and activated data set.");
    return 1;
}

// Deactivates and destroys the datasets (returns 1 on success, 0 on fail)
JNIEXPORT int JNICALL
Java_edu_ethz_s3d_S3D_destroyTrackerData(JNIEnv *, jobject)
{
    LOG("Java_edu_ethz_s3d_S3D_destroyTrackerData");

    // Get the image tracker
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::ImageTracker* imageTracker = static_cast<QCAR::ImageTracker*>(trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER));
    if (imageTracker == NULL) {
        LOG("Failed to destroy the tracking data set because the ImageTracker has not been initialized.");
        return 0;
    }
    
    // Destroy "Stones and Chips" if loaded
    if (dataSetStonesAndChips != 0) {
        if (imageTracker->getActiveDataSet() == dataSetStonesAndChips && !imageTracker->deactivateDataSet(dataSetStonesAndChips)) {
            LOG("Failed to destroy the tracking data set StonesAndChips because the data set could not be deactivated.");
            return 0;
        }

        if (!imageTracker->destroyDataSet(dataSetStonesAndChips)) {
            LOG("Failed to destroy the tracking data set StonesAndChips.");
            return 0;
        }

        LOG("Successfully destroyed the data set StonesAndChips.");
        dataSetStonesAndChips = 0;
    }

    // Destroy "Tarmac" if loaded
    if (dataSetTarmac != 0)
    {
        if (imageTracker->getActiveDataSet() == dataSetTarmac && !imageTracker->deactivateDataSet(dataSetTarmac)) {
            LOG("Failed to destroy the tracking data set Tarmac because the data set could not be deactivated.");
            return 0;
        }

        if (!imageTracker->destroyDataSet(dataSetTarmac)) {
            LOG("Failed to destroy the tracking data set Tarmac.");
            return 0;
        }

        LOG("Successfully destroyed the data set Tarmac.");
        dataSetTarmac = 0;
    }

    return 1;
}

// Registers callback for dataset swap and could set hints for 2 targets
JNIEXPORT void JNICALL
Java_edu_ethz_s3d_S3D_onQCARInitializedNative(JNIEnv *, jobject)
{
    // Register the update callback where we handle the data set swap:
    QCAR::registerCallback(&updateCallback);

    // Comment in to enable tracking of up to 2 targets simultaneously and
    // split the work over multiple frames:
    // QCAR::setHint(QCAR::HINT_MAX_SIMULTANEOUS_IMAGE_TARGETS, 2);
    // QCAR::setHint(QCAR::HINT_IMAGE_TARGET_MULTI_FRAME_ENABLED, 1);
}

// Does all the rendering stuff
JNIEXPORT void JNICALL
Java_edu_ethz_s3d_S3DRenderer_renderFrame(JNIEnv *, jobject)
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

    // Clear color and depth buffer 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Get the state from QCAR and mark the beginning of a rendering section
    QCAR::State state = QCAR::Renderer::getInstance().begin();
    
    // Explicitly render the Video Background
    QCAR::Renderer::getInstance().drawVideoBackground();
       
#ifdef USE_OPENGL_ES_1_1
    // Set GL11 flags:
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
        
#endif

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // TODO: Only work, when there is exactly one trackable
    // Did we find any trackables this frame?
    for(int tIdx = 0; tIdx < state.getNumActiveTrackables(); tIdx++)
    {
        // Get the trackable:
        const QCAR::Trackable* trackable = state.getActiveTrackable(tIdx);
        QCAR::Matrix44F modelViewMatrix = QCAR::Tool::convertPose2GLMatrix(trackable->getPose());

        // Choose the texture based on the target name:
        int textureIndex;
        if (strcmp(trackable->getName(), "chips") == 0)
        {
            textureIndex = 0;
        }
        else if (strcmp(trackable->getName(), "stones") == 0)
        {
            textureIndex = 1;
        }
        else
        {
            textureIndex = 2;
        }

        const Texture* const thisTexture = textures[textureIndex];

#ifdef USE_OPENGL_ES_1_1
        // Load projection matrix:
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(projectionMatrix.data);

        // Load model view matrix:
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(modelViewMatrix.data);
        glTranslatef(0.f, 0.f, kObjectScale);
        glScalef(kObjectScale, kObjectScale, kObjectScale);

        // Draw object:
        glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
        glTexCoordPointer(2, GL_FLOAT, 0, (const GLvoid*) &teapotTexCoords[0]);
        glVertexPointer(3, GL_FLOAT, 0, (const GLvoid*) &teapotVertices[0]);
        glNormalPointer(GL_FLOAT, 0,  (const GLvoid*) &teapotNormals[0]);
        glDrawElements(GL_TRIANGLES, NUM_TEAPOT_OBJECT_INDEX, GL_UNSIGNED_SHORT,
                       (const GLvoid*) &teapotIndices[0]);
#else

        // Calculate the projection matrix
        QCAR::Matrix44F modelViewProjection;
        SampleUtils::translatePoseMatrix(0.0f, 0.0f, kObjectScale, &modelViewMatrix.data[0]);
        SampleUtils::scalePoseMatrix(kObjectScale, kObjectScale, kObjectScale, &modelViewMatrix.data[0]);
        SampleUtils::multiplyMatrix(&projectionMatrix.data[0], &modelViewMatrix.data[0], &modelViewProjection.data[0]);

        // Calculate the camera position
        QCAR::Matrix44F inverseModelView = SampleMath::Matrix44FTranspose(SampleMath::Matrix44FInverse(modelViewMatrix));
        QCAR::Vec3F cameraPosition(inverseModelView.data[12], inverseModelView.data[13], inverseModelView.data[14]);

        // Select the shader program
        glUseProgram(shaderProgramID);

        // Load the vertex attributes
        glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) &teapotVertices[0]);
        glVertexAttribPointer(vertexColorHandle,3,GL_FLOAT, GL_FALSE, 0, (const GLvoid*) &color[0]);
        glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) &teapotNormals[0]);

        // Enable the vertex attributes
        glEnableVertexAttribArray(vertexHandle);
        glEnableVertexAttribArray(normalHandle);
        glEnableVertexAttribArray(vertexColorHandle);
        
        // Load the reconstruction data
        if (reconstructionHandler != NULL) {
        	// First delete the old texture
        	glDeleteTextures(1, &texId);
        	// Load the reconstruction texture and its corresponding data
        	texId = reconstructionHandler->getTexture();
            float slices[2] = { reconstructionHandler->getOverX(), reconstructionHandler->getOVerY() };
            glUniform2fv(glGetUniformLocation(shaderProgramID, "slicesOver"), 1, (const GLfloat*) slices);
            glUniform1f(glGetUniformLocation(shaderProgramID, "numberOfSlices"), reconstructionHandler->getNSlices());
        }
        else {
        	// Load the texture specifications for the white texture
            float slices[2] = { 2.f, 1.f };
            glUniform2fv(glGetUniformLocation(shaderProgramID, "slicesOver"), 1, (const GLfloat*) slices);
            glUniform1f(glGetUniformLocation(shaderProgramID, "numberOfSlices"), 2);
        }

        // Load the eye position into the shader program
        glUniform3fv(glGetUniformLocation(shaderProgramID, "eyePos"), 1, (const GLfloat*) &cameraPosition.data[0]);

        // Load the texture into texture0 and set uVolData to 0 to load it
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texId);
        glUniform1i(glGetUniformLocation(shaderProgramID, "uVolData"), 0);

        // Load the projection matrix into its uniform
        glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
                           (GLfloat*)&modelViewProjection.data[0] );

        // Draw the cube
        glDrawElements(GL_TRIANGLES, NUM_TEAPOT_OBJECT_INDEX, GL_UNSIGNED_SHORT,
                       (const GLvoid*) &teapotIndices[0]);

        SampleUtils::checkGlError("ImageTargets renderFrame");
#endif

    }

    glDisable(GL_DEPTH_TEST);

#ifdef USE_OPENGL_ES_1_1        
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
#else
    glDisableVertexAttribArray(vertexHandle);
    glDisableVertexAttribArray(normalHandle);
    glDisableVertexAttribArray(vertexColorHandle);
#endif

    QCAR::Renderer::getInstance().end();

    glDisable(GL_BLEND);
}


void configureVideoBackground()
{
	QCAR::setFrameFormat(QCAR::RGB888, true);
    // Get the default video mode:
    QCAR::CameraDevice& cameraDevice = QCAR::CameraDevice::getInstance();
    QCAR::VideoMode videoMode = cameraDevice.
                                getVideoMode(QCAR::CameraDevice::MODE_DEFAULT);


    // Configure the video background
    QCAR::VideoBackgroundConfig config;
    config.mEnabled = true;
    config.mSynchronous = true;
    config.mPosition.data[0] = 0.0f;
    config.mPosition.data[1] = 0.0f;
    
    if (isActivityInPortraitMode)
    {
        //LOG("configureVideoBackground PORTRAIT");
        config.mSize.data[0] = videoMode.mHeight
                                * (screenHeight / (float)videoMode.mWidth);
        config.mSize.data[1] = screenHeight;

        if(config.mSize.data[0] < screenWidth)
        {
            LOG("Correcting rendering background size to handle missmatch between screen and video aspect ratios.");
            config.mSize.data[0] = screenWidth;
            config.mSize.data[1] = screenWidth * 
                              (videoMode.mWidth / (float)videoMode.mHeight);
        }
    }
    else
    {
        //LOG("configureVideoBackground LANDSCAPE");
        config.mSize.data[0] = screenWidth;
        config.mSize.data[1] = videoMode.mHeight
                            * (screenWidth / (float)videoMode.mWidth);

        if(config.mSize.data[1] < screenHeight)
        {
            LOG("Correcting rendering background size to handle missmatch between screen and video aspect ratios.");
            config.mSize.data[0] = screenHeight
                                * (videoMode.mWidth / (float)videoMode.mHeight);
            config.mSize.data[1] = screenHeight;
        }
    }

    LOG("Configure Video Background : Video (%d,%d), Screen (%d,%d), mSize (%d,%d)", videoMode.mWidth, videoMode.mHeight, screenWidth, screenHeight, config.mSize.data[0], config.mSize.data[1]);

    // Set the config:
    QCAR::Renderer::getInstance().setVideoBackgroundConfig(config);
}

// Sets screen dimensions, gets the texture count, loads and stores them
JNIEXPORT void JNICALL
Java_edu_ethz_s3d_S3D_initApplicationNative(JNIEnv* env, jobject obj, jint width, jint height)
{
    LOG("Java_edu_ethz_s3d_S3D_initApplicationNative");
    
    // Store screen dimensions
    screenWidth = width;
    screenHeight = height;
        
    // Handle to the activity class:
    jclass activityClass = env->GetObjectClass(obj);

    jmethodID getTextureCountMethodID = env->GetMethodID(activityClass,
                                                    "getTextureCount", "()I");
    if (getTextureCountMethodID == 0)
    {
        LOG("Function getTextureCount() not found.");
        return;
    }

    textureCount = env->CallIntMethod(obj, getTextureCountMethodID);    
    if (!textureCount)
    {
        LOG("getTextureCount() returned zero.");
        return;
    }

    textures = new Texture*[textureCount];

    jmethodID getTextureMethodID = env->GetMethodID(activityClass,
        "getTexture", "(I)Ledu/ethz/s3d/Texture;");

    if (getTextureMethodID == 0)
    {
        LOG("Function getTexture() not found.");
        return;
    }

    // Register the textures
    for (int i = 0; i < textureCount; ++i)
    {

        jobject textureObject = env->CallObjectMethod(obj, getTextureMethodID, i); 
        if (textureObject == NULL)
        {
            LOG("GetTexture() returned zero pointer");
            return;
        }

        textures[i] = Texture::create(env, textureObject);
    }
    LOG("Java_edu_ethz_s3d_S3D_initApplicationNative finished");
}

// Deletes the textures
JNIEXPORT void JNICALL
Java_edu_ethz_s3d_S3D_deinitApplicationNative(JNIEnv* env, jobject obj)
{
    LOG("Java_edu_ethz_s3d_S3D_deinitApplicationNative");

    // Release texture resources
    if (textures != 0)
    {    
        for (int i = 0; i < textureCount; ++i)
        {
            delete textures[i];
            textures[i] = NULL;
        }
    
        delete[]textures;
        textures = NULL;
        
        textureCount = 0;
    }
}

//Starts the camera module and starts the imageTracker
// Dependency: Calls configureVideoBackground
JNIEXPORT void JNICALL
Java_edu_ethz_s3d_S3D_startCamera(JNIEnv *, jobject)
{
    LOG("Java_edu_ethz_s3d_S3D_startCamera");

    // Initialize the camera:
    if (!QCAR::CameraDevice::getInstance().init())
        return;

    // Configure the video background
    configureVideoBackground();

    // Select the default mode:
    if (!QCAR::CameraDevice::getInstance().selectVideoMode(
                                QCAR::CameraDevice::MODE_DEFAULT))
        return;

    // Start the camera:
    if (!QCAR::CameraDevice::getInstance().start())
        return;

    // Uncomment to enable flash
    //if(QCAR::CameraDevice::getInstance().setFlashTorchMode(true))
    //	LOG("IMAGE TARGETS : enabled torch");

    // Uncomment to enable infinity focus mode, or any other supported focus mode
    // See CameraDevice.h for supported focus modes
    //if(QCAR::CameraDevice::getInstance().setFocusMode(QCAR::CameraDevice::FOCUS_MODE_INFINITY))
    //	LOG("IMAGE TARGETS : enabled infinity focus");

    // Start the tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::Tracker* imageTracker = trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER);
    if(imageTracker != 0)
        imageTracker->start();
}

// Stops the tracker and the camera which it also deinits
JNIEXPORT void JNICALL
Java_edu_ethz_s3d_S3D_stopCamera(JNIEnv *, jobject)
{
    LOG("Java_edu_ethz_s3d_S3D_stopCamera");

    // Stop the tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::Tracker* imageTracker = trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER);
    if(imageTracker != 0)
        imageTracker->stop();
    
    QCAR::CameraDevice::getInstance().stop();
    QCAR::CameraDevice::getInstance().deinit();
}

// Gets the camera calibration and sets it as projection matrix
JNIEXPORT void JNICALL
Java_edu_ethz_s3d_S3D_setProjectionMatrix(JNIEnv *, jobject)
{
    LOG("Java_edu_ethz_s3d_S3D_setProjectionMatrix");

    // Cache the projection matrix:
    const QCAR::CameraCalibration& cameraCalibration =
                                QCAR::CameraDevice::getInstance().getCameraCalibration();
    projectionMatrix = QCAR::Tool::getProjectionGL(cameraCalibration, 2.0f,
                                            2000.0f);
}


JNIEXPORT jboolean JNICALL
Java_edu_ethz_s3d_S3D_activateFlash(JNIEnv*, jobject, jboolean flash)
{
    return QCAR::CameraDevice::getInstance().setFlashTorchMode((flash==JNI_TRUE)) ? JNI_TRUE : JNI_FALSE;
}


JNIEXPORT jboolean JNICALL
Java_edu_ethz_s3d_S3D_autofocus(JNIEnv*, jobject)
{
    return QCAR::CameraDevice::getInstance().setFocusMode(QCAR::CameraDevice::FOCUS_MODE_TRIGGERAUTO) ? JNI_TRUE : JNI_FALSE;
}


JNIEXPORT jboolean JNICALL
Java_edu_ethz_s3d_S3D_setFocusMode(JNIEnv*, jobject, jint mode)
{
    int qcarFocusMode;

    switch ((int)mode)
    {
        case 0:
            qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_NORMAL;
            break;
        
        case 1:
            qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_CONTINUOUSAUTO;
            break;
            
        case 2:
            qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_INFINITY;
            break;
            
        case 3:
            qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_MACRO;
            break;
    
        default:
            return JNI_FALSE;
    }
    
    return QCAR::CameraDevice::getInstance().setFocusMode(qcarFocusMode) ? JNI_TRUE : JNI_FALSE;
}


// This attaches the textures to OpenGL
JNIEXPORT void JNICALL
Java_edu_ethz_s3d_S3DRenderer_initRendering(JNIEnv* env, jobject obj)
{
    LOG("Java_edu_ethz_s3d_S3DRenderer_initRendering");

    // Define clear color
    glClearColor(0.0f, 0.0f, 0.0f, QCAR::requiresAlpha() ? 0.0f : 1.0f);
    
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	char texData[32] = {(char)255};
	memset(texData, (char)255, 32);

	// (2D, level 0, internal format, width, height, no border, format, pixel format, data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
#ifndef USE_OPENGL_ES_1_1
  
    shaderProgramID     = SampleUtils::createProgramFromBuffer(cubeMeshVertexShader,
                                                            cubeFragmentShader);

    vertexHandle        = glGetAttribLocation(shaderProgramID,
                                                "vertexPosition");
    normalHandle        = glGetAttribLocation(shaderProgramID,
                                                "vertexNormal");
    mvpMatrixHandle     = glGetUniformLocation(shaderProgramID,
                                                "modelViewProjectionMatrix");

    vertexColorHandle         	= glGetAttribLocation(shaderProgramID,
                                                "aVertexColor");

#endif

}

//sets the screen size
// Dependency: calls configureVideoBackground
JNIEXPORT void JNICALL
Java_edu_ethz_s3d_S3DRenderer_updateRendering(
                        JNIEnv* env, jobject obj, jint width, jint height)
{
    LOG("Java_edu_ethz_s3d_S3DRenderer_updateRendering");

    // Update screen dimensions
    screenWidth = width;
    screenHeight = height;

    // Reconfigure the video background
    configureVideoBackground();
}

#ifdef __cplusplus
}
#endif

