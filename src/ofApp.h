//--------------------------------------------------------------
//* Name: Shadowing.h
//* Project: Playable City 2014 Award
//* Author: David Haylock
//* Creation Date: 30-07-2014
//* Copyright: (c) 2014 by Watershed Arts Trust Ltd.
//--------------------------------------------------------------

// #pragma once

// #include "ofxGifEncoder.h"
// #include "ofxIO.h"
#include "ofxHttpUtils.h"
// #include "ofxSimpleTimer.h"
// #include "ofxUI.h"
// #include "ofxHttpUtils.h"
#include "ofMain.h"
#include "ofxXmlPoco.h"
#include "OCV.h"

//Projctor control needs to be after OCV so that delcarations don't interfere
//I guess namespaces would be good he
#include "ofxProjectorControl.h"

#include "ofxVideoBuffers.h"
// #include "VideoBufferStorage.h"
#include "ofxBlur.h"

#define CAM_WIDTH 320
#define CAM_HEIGHT 240
#define FRAMERATE 25
#define MIN_BUFFER_SIZE 30 //Frames
#define MAX_BUFFER_SIZE 900 //Frames
#define STATUS_FREQUENCY (1000*60)*5

#define SAVE_PATH_NUC "/root/of_v0.8.3_linux64_release/apps/myApps/ShadowingApp/bin/data/gifs/"
#define HAVE_WEB
#define NUC
// #define DEBUG

// using namespace ofx::IO;
class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

        void exit();
        void setupVariables();

        // Important
        bool isProductionMode;

        // Projector Commands
        void setupProjector();
//        ofxProjectorControl projector;

        // OPEN CV
        void setupCV();
        CV openCV;
        bool doCalibration;
        bool learnBackground;
        bool bProgressiveLearning;
        int  threshold;
        int  moveThreshold;
	    float fProgressiveRate;
        bool bUseApprox;
        bool bFillHoles;
        int iMinBlobSize;
        int iMaxBlobSize;
        int iMaxBlobNum;
        float brightness,contrast;
        bool bMirrorH;
        bool bMirrorV;
        float fBlur;
        int gaussBlur;
	    int medianBlur;
	    bool erode;
        bool dilate;
        vector<ofVec2f> blobPath;
  	    long recTimer;

        // GUI
        void setupGUI();
        void setupSimpleGUI();
	    void guiEvent(ofxUIEventArgs &e);
        ofxUICanvas *gui;
        ofImage *colorSampler;

        // Masks
        void setupMasks();
        bool drawMask;
        int whichMask;
        ofImage masks[6];
        vector <string> maskString;
        ofColor backColor, shadowColor;

        // GIF Stuff
        // void setupGifEncoder();
        // int progress;
        // void onGifSaved(string & fileName);
        // void captureFrame();
        // ofxGifEncoder gifEncoder;
        // bool canSaveGif;
        // void playSlowShadow();

        // Shadowing Dream States
        void ShadowingDreamStateB();
        bool dream;

        // void ShadowingDreamStateB();

        // Shadowing Modes
        // bool triggerDreamTimer;
        // long dreamTimer;
// 
        // New Modes
        void ShadowingProductionModeA();
        // void ShadowingProductionModeB();
        // void ShadowingProductionModeG();
        // void ShadowingProductionTest();

        // Old Modes
        // void ShadowingModeA();
        // void ShadowingModeB();
        // void ShadowingModeC();
        // void ShadowingModeD();
        // void ShadowingModeE();
        // void ShadowingModeF();
        // void ShadowingModeG();
        // void ShadowingModeH();
        // void ShadowingDefaultMode();
        // void ShadowingDefaultModeNoLoop();
        // void ShadowingDefaultMirroredMode();

        std::deque<videoBuffer> buffers;
        std::deque<videoBuffer>livebuffer;
        videoBuffer vidBuffer;

        vector<ofImage> videoImage;

        string modeString;
        int playbackMode;
	    int imagingMode;
        int howManyBuffersToStore;
        bool showPreviousBuffers;
        int whichBufferAreWePlaying;
        bool showMultipleBuffers;
        bool startRecording;
        bool drawCV;
        bool drawLiveImage;
        bool hasBeenPushedFlag;
        int liveShadowProgress;

        // Debug Stuff
        void drawData();
        bool canDrawData;
        bool cursorDisplay;
	    bool drawCamFull;
        // Activity,Timers and Modes
        // void setupTimers();
        // bool inactive;
        
        long dreamTimer;
        long statusTimer;
        void sendStatus();
        // ofxSimpleTimer dreamTimer;
        // void dreamTimerComplete();
        // void dreamTimerStarted();

        // ofxSimpleTimer doCVBackgroundTimer;
        // void CVTimerComplete(int &args);
        // void CVTimerStarted(int &args);

        // ofxSimpleTimer statusTimer;
        // void statusTimerComplete();
        // void statusTimerStarted();


        string CVstring;
        void drawMisc();

        int dreamWaitTime;

        // Directory Watcher
        void setupDirectoryWatcher();
        // DirectoryWatcherManager gifWatcher;
        // HiddenFileFilter fileFilter;
        string latestGifPath;
        
        // void onDirectoryWatcherItemAdded(const DirectoryWatcherManager::DirectoryEvent& evt);
        // void onDirectoryWatcherItemRemoved(const DirectoryWatcherManager::DirectoryEvent& evt);
        // void onDirectoryWatcherItemModified(const DirectoryWatcherManager::DirectoryEvent& evt);
        // void onDirectoryWatcherItemMovedFrom(const DirectoryWatcherManager::DirectoryEvent& evt);
        // void onDirectoryWatcherItemMovedTo(const DirectoryWatcherManager::DirectoryEvent& evt);
        // void onDirectoryWatcherError(const Poco::Exception& exc);
        void cleanGifFolder();

        // HTTP
        void setupHTTP();
        void newResponse(ofxHttpResponse & response);
        ofxHttpUtils httpUtils;
     
        string responseStr;
        string requestStr;
        string action_url;

        int imageCounter;
        int howmanyrecordings;

        // Blur Shader
        void setupShader();
        ofxBlur shader;
        bool useShader;
        int passNumber;
        int shaderRadius;
        ofFbo mainOut;
        bool stopLoop;
        int dreamState;
        bool bSwitch;
        int playCounter;
        bool firstLearn;
        bool noneDream;
        bool randomWaitLatch;
        long randomWaitTimer;
        bool playBackLatch;
        bool lastPresentState;


        // New Loadconfig for AWS
        void loadConfig();
        ofxXmlSettings XML;
        string _locationID;
        string _secretKey;
        string _statusurl;
        string _uploadFileURL;

    	//Tiger Videos
    	// void playTiger();
    	// ofVideoPlayer tiger1;
    	// ofVideoPlayer tiger2;
    	// bool		whichTiger;
    	// float 		tigerProbability;

    	//Playback Offset
    	int playbackOffsetY;
    	
    	//Video Playback Delay
    	int delayFramesBeforePlayback;

    	//Mask Scale
    	int maskScale;
    	int maskCenterX;
    	int maskCenterY;

    	//Logging
    	std::ofstream outfile;

};
