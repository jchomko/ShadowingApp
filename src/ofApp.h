//--------------------------------------------------------------
//* Name: Shadowing.h
//* Project: Playable City 2014 Award
//* Author: David Haylock
//* Creation Date: 30-07-2014
//* Copyright: (c) 2014 by Watershed Arts Trust Ltd.
//--------------------------------------------------------------

#pragma once

#include "ofMain.h"
#include "OCV.h"
#include "ofxSimpleTimer.h"
#include "ofxUI.h"
#include "VideoBufferStorage.h"
#include "ofxHttpUtils.h"
#include "ofxProjectorControl.h"
#include "ofxGifEncoder.h"
#include "ofxIO.h"
#include "ofxHttpUtils.h"
#include "ofxBlur.h"

#define CAM_WIDTH 320
#define CAM_HEIGHT 240
#define FRAMERATE 30
#define MIN_BUFFER_SIZE 30 //Frames
#define MAX_BUFFER_SIZE 300 //Frames
#define STATUS_TIMER (1000*60)*2

//--------------------------------------------------------------
//* 1 = Wilder Street
//* 2 = Cathedral Lane
//* 3 = Tower Lane
//* 4 = Champions Square
//* 5 = Station Road
//* 6 = Willmott Park
//* 7 = Leonard Street
//* 8 = Lower Ashley Road
//* 9 = Dame Emily Park
//--------------------------------------------------------------
#define WHERE_ARE_WE "4"

#define UPLOAD_SCRIPT "http://wcode.org/sandbox/shadowing/upload.php"
#define STATUS_SCRIPT "http://wcode.org/sandbox/shadowing/status.php"
#define SECRET_KEY "7dshtbfydjsgMXfdgshj"

#define SAVE_PATH_NUC "/root/of_v0.8.3_linux64_release/apps/myApps/ShadowingApp/bin/data/gifs/"
#define SAVE_PATH_MAC "/Users/davidhaylock/Desktop/of_v0.8.3_osx_release/apps/Shadowing/ShadowingStage0_5/bin/data/gifs/"

//#define DEBUG_MODE
#define NUC

using namespace ofx::IO;
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
        ofxProjectorControl projector;
    
        // OPEN CV
        void setupCV();
        CV openCV;
        bool doCalibration;
        bool learnBackground;
        bool bProgressiveLearning;
        int  threshold;
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
        bool erode;
        bool dilate;
        vector<ofVec2f> blobPath;
    
        // GUI
        void setupGUI();
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
        void setupGifEncoder();
        int progress;
        void onGifSaved(string & fileName);
        void captureFrame();
        ofxGifEncoder gifEncoder;
        bool canSaveGif;
        void playSlowShadow();
    
        // Shadowing Dream States
        void ShadowingDreamStateA();
        void ShadowingDreamStateB();
    
        // Shadowing Modes
        bool dream;
        bool triggerDreamTimer;
    
        // New Modes
        void ShadowingProductionModeA();
        void ShadowingProductionModeB();
        void ShadowingProductionModeG();
        void ShadowingProductionTest();
    
        // Old Modes
        void ShadowingModeA();
        void ShadowingModeB();
        void ShadowingModeC();
        void ShadowingModeD();
        void ShadowingModeE();
        void ShadowingModeF();
        void ShadowingModeG();
        void ShadowingModeH();
        void ShadowingDefaultMode();
        void ShadowingDefaultModeNoLoop();
        void ShadowingDefaultMirroredMode();
    
        std::deque<videoBuffer> buffers;
        std::deque<videoBuffer>livebuffer;
        videoBuffer b;

        vector<ofImage> videoImage;
    
        string modeString;
        int playbackMode;
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
        // Activity,Timers and Modes
        void setupTimers();
        bool inactive;
        ofxSimpleTimer activityTimer;
        void activityTimerComplete(int &args);
        void activityTimerStarted(int &args);
    
        ofxSimpleTimer doCVBackgroundTimer;
        void CVTimerComplete(int &args);
        void CVTimerStarted(int &args);
    
        ofxSimpleTimer statusTimer;
        void statusTimerComplete(int &args);
        void statusTimerStarted(int &args);
    
        string CVstring;
        void drawMisc();
    
        // Directory Watcher
        void setupDirectoryWatcher();
        DirectoryWatcherManager gifWatcher;
        HiddenFileFilter fileFilter;
        string latestGifPath;
        void onDirectoryWatcherItemAdded(const DirectoryWatcherManager::DirectoryEvent& evt);
        void onDirectoryWatcherItemRemoved(const DirectoryWatcherManager::DirectoryEvent& evt);
        void onDirectoryWatcherItemModified(const DirectoryWatcherManager::DirectoryEvent& evt);
        void onDirectoryWatcherItemMovedFrom(const DirectoryWatcherManager::DirectoryEvent& evt);
        void onDirectoryWatcherItemMovedTo(const DirectoryWatcherManager::DirectoryEvent& evt);
        void onDirectoryWatcherError(const Poco::Exception& exc);
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
};

