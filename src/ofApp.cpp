//--------------------------------------------------
//* Name: Shadowing.cpp
//* Project: Playable City 2014 Award
//* Author: David Haylock
//* Creation Date: 30-07-2014
//* Copyright: (c) 2014 by Watershed Arts Trust Ltd.
//--------------------------------------------------
#include "ofApp.h"
//--------------------------------------------------------------
void ofApp::setup()
{
    ofBuffer buffer = ofBufferFromFile("/root/id.txt");
    cout << buffer.getText();
    unitID = buffer.getText();
    ofSetWindowShape(ofGetScreenWidth(), ofGetScreenHeight());
    ofSetFrameRate(FRAMERATE);
    ofSetVerticalSync(true);
    // Setup the Projector
    setupProjector();
    
    // Setup Variables
    setupVariables();
    
    // Setup Gif Encoder
    setupGifEncoder();
    
    // Setup Directory Watcher
    setupDirectoryWatcher();
    
    // Setup HTTP Stuff
    setupHTTP();
    
    // Setup CV
    setupCV();
    
    // Looks for masks inside of the Masks folder
    setupMasks();
    
    // Setup the GUI
    setupGUI();

    // Setup the Timer
    setupTimers();
    
    // Setup Shaders
    setupShader();
    playBackLatch = false;
    lastPresentState = false;    
}
//--------------------------------------------------------------
void ofApp::update()
{
    // Set Window Title
    string title = "Shadowing: " + ofToString(ofGetTimestampString("%H:%M:%S  %d/%m/%Y"));
    ofSetWindowTitle(title);
    
    //--------------------------------------------------------------
    // If we have %i buffers in the memory then release one
    if (buffers.size() > howManyBuffersToStore)
    {
        buffers.pop_back();
    }
    
    if (livebuffer.size() > 2) {
        //buffers.push_front(livebuffer[1]);
        livebuffer.pop_back();
    }
    
    //--------------------------------------------------------------
    // Custom CV mechanisms
    // Wait until we have a new frame before learning background
    if ( openCV.newFrame() && firstLearn == true)
    {
        openCV.relearnBackground();
        firstLearn = false;
    }
    
    // Subtraction Plus Brightness and Contrast Settings
openCV.JsubtractionLoop(learnBackground, bMirrorH,bMirrorV,threshold,fBlur,iMinBlobSize, iMaxBlobSize,iMaxBlobNum,bFillHoles,bUseApprox,brightness,contrast);
    
    
//openCV.subtractionLoop(learnBackground, bProgressiveLearning,fProgressiveRate,bMirrorH,bMirrorV,threshold,fBlur,iMinBlobSize, iMaxBlobSize,iMaxBlobNum,bFillHoles,bUseApprox,erode,dilate);
    learnBackground = false;
    // Do Blob Assembly
    openCV.readAndWriteBlobData(ofColor::white,ofColor::black);

    // Can choose the background color
    //openCV.readAndWriteBlobData(backColor,shadowColor);
    

    // If blob detected Start Recording
    if(openCV.isSomeoneThere() && imageCounter < MAX_BUFFER_SIZE)
    {
	if (buffers.size() > 3)
	{
		whichBufferAreWePlaying = 2;
	}
	else
	{
		whichBufferAreWePlaying = 0;
	}

        canSaveGif = true;
        activityTimer.stop();
        doCVBackgroundTimer.stop();
        startRecording = true;
        hasBeenPushedFlag = false;
        dream = false;
	noneDream = false;
        triggerDreamTimer = true;
    }
    else
    {
        bSwitch = true;
        startRecording = false;
        if (hasBeenPushedFlag == false)
        {
            if (imageCounter >= MIN_BUFFER_SIZE)
            {
                if (canSaveGif == true)
                {
                    #ifdef NUC
                    gifEncoder.save(SAVE_PATH_NUC+ofGetTimestampString()+".gif");
                    #else
                    gifEncoder.save(SAVE_PATH_MAC+ofGetTimestampString()+".gif");
                    #endif
                    howmanyrecordings++;
                    canSaveGif = false;
                }
		buffers.push_front(b);            
                b.clear();
                hasBeenPushedFlag = true;
                imageCounter = 0;
                doCVBackgroundTimer.start(false);
            }
            else if(imageCounter < MIN_BUFFER_SIZE)
            {
                b.clear();
                imageCounter = 0;
                hasBeenPushedFlag = true;
            }
            else
            {
             
            }
        }
        else
        {
            //Do nothing
        }
    }
    
    if(startRecording == true)
    {
        // If new frame
        if (openCV.newFrame())
        {
            // Capture Gif Image every 5 frames
            if (ofGetFrameNum() % 5 == 0)
            {
                captureFrame();
            }
            // Capture Data according to %i number
            if (ofGetFrameNum() % 1 == 0)
            {
                // Capture the CV image
                b.getNewImage(openCV.getRecordPixels());
                blobPath.push_back(openCV.getBlobPath());
                imageCounter++;
            }
        }
    }
    else if (!startRecording)
    {
        
    }
    
    if(!openCV.isSomeoneThere())
    {
        if (canSaveGif == true)
        {
            #ifdef NUC
                gifEncoder.save(SAVE_PATH_NUC+ofGetTimestampString()+".gif");
            #else
                gifEncoder.save(SAVE_PATH_MAC+ofGetTimestampString()+".gif");
            #endif
            howmanyrecordings++;
            canSaveGif = false;
        }
        stopLoop = true;
    }
    
    if (!openCV.isSomeoneThere() && triggerDreamTimer == true)
    {
        activityTimer.start(false);
        triggerDreamTimer = false;
    }
    
    // Update the buffer progressors
    if (!buffers.empty())
    {
        for (int i = 0; i < buffers.size(); i++)
        {
            buffers[i].update();
        }
    }
    
    // Update the Holding buffer progressors
    if (!livebuffer.empty())
    {
        for (int i = 0; i < livebuffer.size(); i++)
        {
            livebuffer[i].update();
        }
    }
    
    doCVBackgroundTimer.update();
    statusTimer.update();
    activityTimer.update();
    
    if (cursorDisplay == true)
    {
        ofShowCursor();
    }
    else
    {
        ofHideCursor();
    }
}
//--------------------------------------------------------------
void ofApp::draw()
{
    ofBackground(backColor);
    mainOut.begin();
    ofClear(backColor);
    
    if (useShader)
    {
        shader.begin();
        ofSetColor(255, 255);
        ofRect(0, 0, 320,240);
    }
    ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);   
    if (!livebuffer.empty())
    {
        for (int i = 0; i < livebuffer.size(); i++)
        {
            livebuffer[i].draw(255);
        }
    }
    if (!buffers.empty())
    {
        for (int i = 0; i < buffers.size(); i++)
        {
            buffers[i].draw(255);
        }
    }
    ofDisableBlendMode();
   
    if (playbackMode == 0)
    {
        ShadowingProductionModeA();
    }
    else if (playbackMode == 1)
    {
	ShadowingProductionModeA();
    }
    else if (playbackMode == 2)
    {

    }
    else
    {
	
    }
    
    if (useShader)
    {
        shader.end();
        shader.draw();
    }
    mainOut.end();

//-------------Main Drawing Mechanism-----------
//----------------------------------------------
//----------------------------------------------
    ofSetColor(255, 255, 255);
    mainOut.draw(0,0,ofGetWidth(),ofGetHeight());
//----------------------------------------------
//----------------------------------------------
    
    
    // As it implies does alpha layering and draws mask to blur the edges of the projection
    if (drawMask)
    {
        ofEnableAlphaBlending();
        ofSetColor(255, 255);
        masks[whichMask].draw(0,0,ofGetWidth(),ofGetHeight());
        ofDisableAlphaBlending();
    }
    drawMisc();
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    switch(key)
    {
        case '1':
	    playbackMode = 0;
	    break;
	case '2':
	    playbackMode = 1;
	    break;
	case '3':
	    playbackMode = 3;
	    break;
	case 'h':
            gui->toggleVisible();
            break;
        case 'm':
            projector.menu();
            break;
        case OF_KEY_UP:
            projector.up();
            break;
        case OF_KEY_DOWN:
            projector.down();
            break;
        case OF_KEY_LEFT:
            projector.left();
            break;
        case OF_KEY_RIGHT:
            projector.right();
            break;
        case 'i':
            projector.projectorOn();
            break;
        case 'o':
            projector.projectorOff();
            break;
        case 'c':
            cursorDisplay = !cursorDisplay; // NULL on the ubuntu system
            break;
        case 'd':
            canDrawData = !canDrawData;
            ((ofxUILabelToggle *) gui->getWidget("Show Data"))->setValue(canDrawData);
            break;
        case 'v':
            drawCV = !drawCV;
            ((ofxUILabelToggle *) gui->getWidget("Draw CV"))->setValue(drawCV);
            break;
        case 'b':
            showPreviousBuffers = !showPreviousBuffers;
            ((ofxUILabelToggle *) gui->getWidget("Show Buffers"))->setValue(showPreviousBuffers);
            break;
        case 't':
            // Send the Gif to the Server
            ofxHttpForm form;
            form.action = STATUS_SCRIPT;
            form.method = OFX_HTTP_POST;
            form.addFormField("secret", SECRET_KEY);
            form.addFormField("location", unitID);
            form.addFormField("status", "TESTEVENT");
            form.addFormField("numberofrecordings", ofToString(howmanyrecordings));
            form.addFormField("submit","1");
            httpUtils.addForm(form);
            break;
    }
}
//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{

}
//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y )
{

}
//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{
    if (doCalibration)
    {
        openCV.mouseDragged(x, y, button);
    }
}
//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
    if (doCalibration)
    {
        openCV.mousePressed(x, y, button);
    }
}
//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
    if (doCalibration)
    {
        openCV.mouseReleased(x, y, button);
    }
}
//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{

}
//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg)
{

}
//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{

}
//--------------------------------------------------------------
void ofApp::newResponse(ofxHttpResponse & response)
{
	responseStr = ofToString(response.status) + ":" + (string)response.responseBody;
}
//--------------------------------------------------------------
void ofApp::onDirectoryWatcherItemAdded(const DirectoryWatcherManager::DirectoryEvent& evt)
{
#ifndef NUC
    if (ofIsStringInString(evt.item.path(),".gif") && evt.item.getSize() > 5000)
    {
        cout << "File: " <<  latestGifPath << " Size: " << evt.item.getSize() <<" bytes" << endl;
        latestGifPath = evt.item.path();
        
        // Send the Gif to the Server
        ofxHttpForm form;
        form.action = UPLOAD_SCRIPT;
        form.method = OFX_HTTP_POST;
        form.addFormField("secret", SECRET_KEY);
        form.addFormField("location", unitID);
        form.addFormField("timestamp",ofGetTimestampString());
        form.addFormField("currentmode", modeString);
        form.addFile("file",latestGifPath);
        form.addFormField("submit","1");
        httpUtils.addForm(form);
    }
#endif
}
//--------------------------------------------------------------------------------------------------
void ofApp::onDirectoryWatcherItemRemoved(const DirectoryWatcherManager::DirectoryEvent& evt){ }
//--------------------------------------------------------------------------------------------------
void ofApp::onDirectoryWatcherItemModified(const DirectoryWatcherManager::DirectoryEvent& evt){ }
//--------------------------------------------------------------------------------------------------
void ofApp::onDirectoryWatcherItemMovedFrom(const DirectoryWatcherManager::DirectoryEvent& evt){ }
//--------------------------------------------------------------------------------------------------
void ofApp::onDirectoryWatcherItemMovedTo(const DirectoryWatcherManager::DirectoryEvent& evt)
{
#ifdef NUC
    if (ofIsStringInString(evt.item.path(),".gif") && evt.item.getSize() > 5000)
    {
        cout << "File: " <<  latestGifPath << evt.item.getSize() << endl;
        latestGifPath = evt.item.path();
        
        // Send the Gif to the Server
        ofxHttpForm form;
        form.action = UPLOAD_SCRIPT;
        form.method = OFX_HTTP_POST;
        form.addFormField("secret", SECRET_KEY);
        form.addFormField("location", unitID);
        form.addFormField("timestamp",ofGetTimestampString());
        form.addFormField("currentmode", modeString);
        form.addFile("file",latestGifPath);
        form.addFormField("submit","1");
        httpUtils.addForm(form);
    }
#endif
    
}
//--------------------------------------------------------------------------------------------------
void ofApp::onDirectoryWatcherError(const Poco::Exception& exc){ }
//--------------------------------------------------------------
void ofApp::exit()
{
    gifWatcher.unregisterAllEvents(this);
    
    // As it says
    cleanGifFolder();
    
    // Send the Gif to the Server
    ofxHttpForm form;
    form.action = STATUS_SCRIPT;
    form.method = OFX_HTTP_POST;
    form.addFormField("secret", SECRET_KEY);
    form.addFormField("location", unitID);
    form.addFormField("status", "STOPPED");
    form.addFormField("numberofrecordings", ofToString(howmanyrecordings));
    form.addFormField("submit","1");
    httpUtils.addForm(form);
    
    ofSleepMillis(5000);
    httpUtils.stop();
 
    gui->saveSettings("GUI/Settings.xml");
    delete gui;
    projector.projectorOff();
}
//--------------------------------------------------------------
//* Other Stuff
//--------------------------------------------------------------
//--------------------------------------------------------------
void ofApp::onGifSaved(string &fileName)
{
    // When the encoding process begins reset gifEncoder for next use
    gifEncoder.reset();
}
//--------------------------------------------------------------
//* Other Stuff
//--------------------------------------------------------------
//--------------------------------------------------------------
void ofApp::captureFrame()
{
    // Yep addFrame to the gifEncoder
    gifEncoder.addFrame(openCV.getRecordPixels().getPixels(),CAM_WIDTH,CAM_HEIGHT,openCV.getRecordPixels().getBitsPerPixel(),0.1f);
}
//--------------------------------------------------------------
//* Clear the gifFolder at the end of the cycle
//--------------------------------------------------------------
//--------------------------------------------------------------
void ofApp::cleanGifFolder()
{
    #ifdef NUC
        ofSystem("rm -rfv /root/of_v0.8.3_linux64_release/apps/myApps/ShadowingApp/bin/data/gifs/*");
    #else
        ofSystem("rm -rfv /Users/davidhaylock/Desktop/of_v0.8.3_osx_release/apps/Shadowing/ShadowingStage0_5/bin/data/gifs/*");
    #endif
}
//--------------------------------------------------------------
//*-------------------------------------------------------------
//* SETUP ROUTINES
//*-------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
//* Setup OpenCV
//--------------------------------------------------------------
void ofApp::setupCV()
{
    // Setup Custom openCV Class
    openCV.setup(CAM_WIDTH,CAM_HEIGHT,FRAMERATE);
    
    // Its a Mystery
    openCV.relearnBackground();
    
    // Sets the internal Tracking boundaries at 40px from each boundary
    openCV.setTrackingBoundaries(40, 40);
}
//--------------------------------------------------------------
//* Setup Variables
//--------------------------------------------------------------
void ofApp::setupVariables()
{
    ofSetLogLevel(OF_LOG_WARNING);
    imageCounter =0;
    playCounter = 0;
    dream = false;
    triggerDreamTimer = false;
    progress = 0;
    playbackMode = 0;
    howmanyrecordings = 0;
    whichBufferAreWePlaying = 0;
    hasBeenPushedFlag = true;
    learnBackground = true;
    canSaveGif = false;
    stopLoop = false;
    bSwitch = false;
    firstLearn = true;
    noneDream == false;
}
//--------------------------------------------------------------
//* Setup Directory Watcher
//--------------------------------------------------------------
void ofApp::setupDirectoryWatcher()
{
#ifdef NUC
    // Add listener
    gifWatcher.registerAllEvents(this);
    
    // Folder to Watch
    std::string gifFolderToWatch = SAVE_PATH_NUC;
    
    // Do not list the existing files as there will be none
    bool listExistingItemsOnStart = false;
    
    gifWatcher.addPath(gifFolderToWatch, listExistingItemsOnStart, &fileFilter);
    
    latestGifPath = "";
#else
    gifWatcher.registerAllEvents(this);
    
    std::string gifFolderToWatch = SAVE_PATH_MAC;
    
    bool listExistingItemsOnStart = false;
    
    gifWatcher.addPath(gifFolderToWatch, listExistingItemsOnStart, &fileFilter);
    
    latestGifPath = "";

#endif
}
//--------------------------------------------------------------
//* Setup HTTP Utils
//--------------------------------------------------------------
void ofApp::setupHTTP()
{
    // Setup HTTP POST Unit
    ofAddListener(httpUtils.newResponseEvent,this,&ofApp::newResponse);
	httpUtils.start();
    
    // Send Status
    ofxHttpForm form;
    form.action = STATUS_SCRIPT;
    form.method = OFX_HTTP_POST;
    form.addFormField("secret", SECRET_KEY);
    form.addFormField("location", unitID);
    form.addFormField("status", "STARTED");
    form.addFormField("numberofrecordings", ofToString(howmanyrecordings));
    form.addFormField("submit","1");
    httpUtils.addForm(form);
}
//--------------------------------------------------------------
//* Setup Gif Encoder
//--------------------------------------------------------------
void ofApp::setupGifEncoder()
{
    gifEncoder.setup(CAM_WIDTH, CAM_HEIGHT, .25, 256);
    ofAddListener(ofxGifEncoder::OFX_GIF_SAVE_FINISHED, this, &ofApp::onGifSaved);
}
//--------------------------------------------------------------
//* Setup Shader
//--------------------------------------------------------------
void ofApp::setupShader()
{
    shader.setup(CAM_WIDTH,CAM_HEIGHT);
    // Setup FBO
    mainOut.allocate(CAM_WIDTH,CAM_HEIGHT,GL_RGBA);
    // Clear FBO
    mainOut.begin();
    ofClear(255);
    mainOut.end();
}
//--------------------------------------------------------------
//* Setup Projector
//--------------------------------------------------------------
void ofApp::setupProjector()
{
    // Connect to the projector
    projector.openConnection("/dev/ttyUSB0");
    
    // Wait while connection is established
    ofSleepMillis(1000);
    
    // Turn the projector On
    projector.projectorOn();
}
//--------------------------------------------------------------
//* Setup Masks
//--------------------------------------------------------------
void ofApp::setupMasks()
{
    // Look inside of the Masks folder
    ofDirectory maskDirectory;
    int nFiles = maskDirectory.listDir("Masks");
    
    // Not sure why I've sorted them?
    maskDirectory.sort();
    
    if(nFiles)
    {
        ofLog(OF_LOG_NOTICE, "Found Mask Folder");
        for(int i = 0; i< maskDirectory.numFiles(); i++)
        {
            string filePath = maskDirectory.getPath(i);
            masks[i].loadImage(filePath);
        }
    }
    else
    {
        ofLog(OF_LOG_ERROR, "Can't Find Mask Folder");
    }
}
//--------------------------------------------------------------
//* Setup Timers
//--------------------------------------------------------------
void ofApp::setupTimers()
{
    statusTimer.setup(STATUS_TIMER); // Every 2 minutes 1000 millis * 60 seconds * 2
    activityTimer.setup(60000);
    doCVBackgroundTimer.setup(5000);
    
    ofAddListener(activityTimer.TIMER_STARTED, this, &ofApp::activityTimerStarted);
    ofAddListener(activityTimer.TIMER_COMPLETE, this, &ofApp::activityTimerComplete);
    
    ofAddListener(doCVBackgroundTimer.TIMER_STARTED, this, &ofApp::CVTimerStarted);
    ofAddListener(doCVBackgroundTimer.TIMER_COMPLETE, this, &ofApp::CVTimerComplete);
    
    ofAddListener(statusTimer.TIMER_STARTED, this, &ofApp::statusTimerStarted);
    ofAddListener(statusTimer.TIMER_COMPLETE, this, &ofApp::statusTimerComplete);
    
    statusTimer.start(true);
}
//--------------------------------------------------------------
//* Setup GUI
//--------------------------------------------------------------
//--------------------------------------------------------------
void ofApp::setupGUI()
{
    colorSampler = new ofImage();
    colorSampler->loadImage("GUI/colorSamplerImage.png");
    
    gui = new ofxUICanvas(ofGetWidth()-260,0,600,600);
    gui->setColorBack(ofColor::black);
    gui->addWidgetDown(new ofxUILabel("Shadowing", OFX_UI_FONT_MEDIUM));
    gui->addSpacer(255,1);
    gui->addWidgetDown(new ofxUILabelToggle("Fullscreen",true,255,30,OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabelToggle("Do Calibration",false,255,30,OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabelToggle("Draw CV",true,255,30,OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabelToggle("Show Buffers",true,255,30,OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabelToggle("Show Data",true,255,30,OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabel("Playback Mode", OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUINumberDialer(0, 12,1, 0, "PLAYBACK_MODE", OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabel("Number of Buffers", OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUINumberDialer(0, 15,5, 0, "BUFFER_NUMBER", OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabelToggle("Use Mask",true,255,30,OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabel("Mask Number", OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUINumberDialer(0, 5, 1, 0, "Mask_No", OFX_UI_FONT_MEDIUM));
    gui->addSpacer(255,1);
    gui->addWidgetDown(new ofxUILabelButton("Learn Background",false,255,30,OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabelToggle("Mirror H",false,255/2,30,OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUILabelToggle("Mirror V",false,255/2,30,OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabel("Tracking Boundaries", OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUINumberDialer(0, CAM_HEIGHT/2,5, 0, "TRACKING_BOUNDARY", OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabel("Threshold", OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUINumberDialer(0, 255, 80, 0, "THRESHOLD", OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabel("Min Blob Size", OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUINumberDialer(0, (CAM_WIDTH*CAM_HEIGHT)/3, 20, 1, "MIN_BLOB_SIZE", OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabel("Max Blob Size", OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUINumberDialer(0, (CAM_WIDTH*CAM_HEIGHT), (CAM_WIDTH*CAM_HEIGHT)/3, 1, "MAX_BLOB_SIZE", OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabel("Max Num Blob", OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUINumberDialer(0, 10, 2, 0, "MAX_BLOB_NUM", OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabelToggle("Fill Holes",false,255/2,30,OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUILabelToggle("Use Approximation",false,255/2,30,OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabel("Blur", OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUINumberDialer(0, 100, 1, 1, "BLUR", OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabel("Brightness", OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUINumberDialer(0, 100, 1, 2, "BrightnessV", OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabel("Contrast", OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUINumberDialer(0, 100, 1, 2, "ContrastV", OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabelToggle("Erode",false,255/2,30,OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUILabelToggle("Dilate",false,255/2,30,OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabelToggle("Progressive Background",false,255,30,OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabel("Progression Rate", OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUINumberDialer(0.00f, 1.00f, 0.01f, 4, "PROGRESSIVE_RATE", OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabelToggle("Use Shader",false,255/2,30,OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabel("Blur Radius", OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUINumberDialer(0, 100, 1, 2, "BLUR_RADIUS", OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabel("Blur Pass", OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUINumberDialer(0, 100, 1, 2, "BLUR_PASS", OFX_UI_FONT_MEDIUM));
    gui->addSpacer(255,1);
    gui->addWidgetDown(new ofxUILabel("BGL","Background Color", OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUIImageSampler(255/2, 255/2, colorSampler, "Background_Color"));
    gui->addWidgetEastOf(new ofxUILabel("Shadow Color", OFX_UI_FONT_MEDIUM),"BGL");
    gui->addWidgetEastOf(new ofxUIImageSampler(255/2, 255/2, colorSampler, "Shadow_Color"),"Background_Color");
    gui->autoSizeToFitWidgets();
    ofAddListener(gui->newGUIEvent,this, &ofApp::guiEvent);

    gui->loadSettings("GUI/Settings.xml");
    gui->setVisible(false);
}
//--------------------------------------------------------------
void ofApp::statusTimerComplete(int &args)
{
    // Send Status to the Server
    ofxHttpForm form;
    form.action = STATUS_SCRIPT;
    form.method = OFX_HTTP_POST;
    form.addFormField("secret", SECRET_KEY);
    form.addFormField("location", unitID);
    form.addFormField("status", "ON");
    form.addFormField("numberofrecordings", ofToString(howmanyrecordings));
    form.addFormField("submit","1");
    httpUtils.addForm(form);
}
//--------------------------------------------------------------
void ofApp::statusTimerStarted(int &args)
{
    
}
//--------------------------------------------------------------
void ofApp::CVTimerStarted(int &args)
{
    CVstring = "CV Timer Started";
}
//--------------------------------------------------------------
void ofApp::CVTimerComplete(int &args)
{
    openCV.relearnBackground();
    CVstring = "CV Timer Done";
}
//--------------------------------------------------------------
void ofApp::activityTimerComplete(int &args)
{
    	cout << "Timer Complete" << endl;
	noneDream = false;
	dream = true;
}
//--------------------------------------------------------------
void ofApp::activityTimerStarted(int &args)
{
    cout << "triggered Timer" << endl;
}
//--------------------------------------------------------------
void ofApp::drawMisc()
{
    // Draw all the CV Stuff
    ofPushStyle();
    if(drawCV == true)
    {
        openCV.draw();
    }
    ofPopStyle();
    
    // Show the previous Buffers
    ofPushStyle();
    if (showPreviousBuffers)
    {
        if (!buffers.empty())
        {
            for (int i = 0; i < buffers.size(); i++)
            {
                // Draw the Mini Buffers
                buffers[i].drawMini(640, 0+(i*240/4));
            }
        }
        if (!livebuffer.empty())
        {
            for (int i = 0; i < livebuffer.size(); i++)
            {
                // Draw the Mini Buffers
                livebuffer[i].drawMini(640+80, 0+(i*240/4));
            }
        }
    }
    ofPopStyle();
    
    // Debug for information about the buffers, openCV HTTP Posts
    if (canDrawData == true)
    {
        drawData();
    }
}
//--------------------------------------------------------------
//* Here are all the Buffer Playback Functions
//--------------------------------------------------------------
//--------------------------------------------------------------
//* Live Delayed Shadow - Participants Shadow a second behind live action
//--------------------------------------------------------------
void ofApp::playSlowShadow()
{
    ofSetColor(255);
    if (!videoImage.empty())
    {
        videoImage[liveShadowProgress].draw(0, 0, ofGetWidth(),ofGetHeight());
    }
    
    if(openCV.isSomeoneThere())
    {
        if (!buffers.empty())
        {
            buffers[0].start();
        }
    }
    
    // Draw the first buffer in the Vector
    if (!buffers.empty())
    {
        if (backColor.getBrightness() >=125)
        {
            ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
        }
        else if (backColor.getBrightness() <= 124)
        {
            ofEnableBlendMode(OF_BLENDMODE_ADD);
        }
        else
        {
            
        }
        openCV.drawLiveShadow();
        buffers[0].draw(255);
        ofDisableBlendMode();
    }
}
//--------------------------------------------------------------
//* SHADOWING STATES AND FUNCTIONS
//--------------------------------------------------------------
//--------------------------------------------------------------
//* THESE ARE THE NEW PLAYBACK STATES
//--------------------------------------------------------------
//--------------------------------------------------------------
//* Dream state - Play all the buffers in memory simultaneously
//* Layered
//--------------------------------------------------------------
void ofApp::ShadowingDreamStateA()
{
    modeString = "Dreaming About Everybody";
    if (!buffers.empty())
    {
        for (int i = 0; i < buffers.size(); i++)
        {
            buffers[i].start();
        }
    }
    
    if (!buffers.empty())
    {
        for (int i = 0; i < buffers.size(); i++)
        {
            if (buffers[i].isFinished())
            {
                buffers[i].reset();
            }
        }
    }
    
    if (!buffers.empty())
    {
        for (int i = 0; i < buffers.size(); i++)
        {
            // This will change the Blend Modes according to the brightness of FBO
            if (backColor.getBrightness() >=125)
            {
                ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
            }
            else if (backColor.getBrightness() <= 124)
            {
                ofEnableBlendMode(OF_BLENDMODE_ADD);
            }
            else
            {
                // Nothing
            }
            
            buffers[i].draw(255);
            ofDisableBlendMode();
        }
    }
}
//--------------------------------------------------------------
//* Dream state - Play the buffers Sequentially
//--------------------------------------------------------------
void ofApp::ShadowingDreamStateB()
{
    modeString = "Dreaming Sequentially";
    // Check if the buffers are live. Start and Draw the first element in the vector.
    // When the buffer has finished playing the iterate to the next buffer
    if (!buffers.empty())
    {
        buffers[whichBufferAreWePlaying].start();
        
        // This will change the Blend Modes according to the brightness of FBO
        if (backColor.getBrightness() >=125)
        {
            ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
        }
        else if (backColor.getBrightness() <= 124)
        {
            ofEnableBlendMode(OF_BLENDMODE_ADD);
        }
        else
        {
            // Nothing
        }
        //buffers[whichBufferAreWePlaying].draw(255);
        ofDisableBlendMode();
        
        if (buffers.size() > 2)
        {
            if (buffers[whichBufferAreWePlaying].isFinished() && randomWaitLatch == false)
            {
                randomWaitTimer = ofGetElapsedTimeMillis() + ofRandom(1000,4000);
		randomWaitLatch = true;
		// Reset the Awaiting buffer otherwise nothing will happen
                // buffers[whichBufferAreWePlaying+1].reset();
                // Progress the Buffer Counter
                // whichBufferAreWePlaying++;
                // buffers[whichBufferAreWePlaying].start();
            }
        }
	if (buffers.size() > 2)
	{
        	if (randomWaitLatch && ofGetElapsedTimeMillis() > randomWaitTimer)
		{
			if (whichBufferAreWePlaying >= buffers.size())
        		{
            			// Reset the first Buffer
	            		buffers[0].reset();
        	    		// Go back to the start and Await my instructions
            			whichBufferAreWePlaying = 0;
           			buffers[whichBufferAreWePlaying].start();
				randomWaitLatch = false;
        		}
			else
			{
				buffers[whichBufferAreWePlaying+1].reset();
				whichBufferAreWePlaying++;
				buffers[whichBufferAreWePlaying].start();
				randomWaitLatch = false;
			}	
		}

	}
	if (whichBufferAreWePlaying >= buffers.size())
	{
		buffers[0].reset();
		whichBufferAreWePlaying = 0;
		buffers[whichBufferAreWePlaying].start();
	}
    }
}
//--------------------------------------------------------------
//* THESE ARE THE NEW PLAYBACK FUNCTIONS
//--------------------------------------------------------------
//--------------------------------------------------------------
//* Default - Walking under light triggers the most recent recording
//* Whilst recording the current shadow (Repeats)
//--------------------------------------------------------------
//--------------------------------------------------------------
void ofApp::ShadowingDefaultMode()
{
    modeString = "Default Shadowing";
    if(openCV.isSomeoneThere())
    {
        if (!buffers.empty())
        {
            buffers[0].start();
            buffers[0].draw(255);
            
            if (buffers[0].isFinished())
            {
                buffers[0].reset();
            }
        }
    }
}
//--------------------------------------------------------------
//* Default No Loop - Walking under light triggers the most recent recording
//* Whilst recording the current shadow (Repeats)
//--------------------------------------------------------------
//--------------------------------------------------------------
void ofApp::ShadowingDefaultModeNoLoop()
{
    modeString = "Default Shadowing No Loop";
    if(openCV.isSomeoneThere())
    {
        if (!buffers.empty())
        {
            buffers[0].start();
            buffers[0].draw(255);
            
            if (buffers[0].isFinished())
            {
                buffers[0].stop();
            }
        }
    }
}
//--------------------------------------------------------------
//* Default Mirrored - Walking under light triggers the most recent recording
//* Whilst recording the current shadow (Repeats)
//--------------------------------------------------------------
//--------------------------------------------------------------
void ofApp::ShadowingDefaultMirroredMode()
{
    modeString = "Default Mirrored Shadowing";
    if(openCV.isSomeoneThere())
    {
        if (!buffers.empty())
        {
            buffers[0].start();
            // Past Version
            buffers[0].draw(255);
            
            if (buffers[0].isFinished())
            {
                buffers[0].reset();
            }
        }
    }
    if (backColor.getBrightness() >=125)
    {
        ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
    }
    else if (backColor.getBrightness() <= 124)
    {
        ofEnableBlendMode(OF_BLENDMODE_ADD);
    }
    else
    {
        
    }
    // This is the Mirrored Version
    ofSetColor(255);
    if (!videoImage.empty())
    {
        ofImage reverseImg;
        reverseImg.setFromPixels(openCV.getRecordPixels());
        reverseImg.mirror(false, true);
        reverseImg.draw(0,0, 320, 240);
    }
    // Live Version
    openCV.drawLiveShadow();
    ofDisableBlendMode();
}
//--------------------------------------------------------------
//* Production BASIC A - Walking under light triggers the most recent recording
//* Then playback the new recorded buffer
//* Then playback the previous buffers sequentially
//* If no users, Do the Dream State
//--------------------------------------------------------------
//--------------------------------------------------------------
void ofApp::ShadowingProductionModeA()
{
// This will change the Blend Modes according to the brightness of FBO
    if (backColor.getBrightness() >= 125)
    {
        ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
    }
    else if (backColor.getBrightness() <= 124)
    {
        ofEnableBlendMode(OF_BLENDMODE_ADD);
    }
    else
    {
        // Nothing
    }
    
    if(openCV.isSomeoneThere() && openCV.isSomeoneThere() != lastPresentState && buffers.size() > 0 && !buffers[0].isPlaying())
    {
        
        playBackLatch = false;
        
        modeString = "Shadowing Basic Mode";
        
        buffers[0].reset();
        buffers[0].start();
        
    }
    else if(!openCV.isSomeoneThere() && dream == false && playBackLatch == false)
    {
        bSwitch = true;
        modeString = "Shadowing Basic Mode Stage 2";
        
        buffers[0].start();
        playBackLatch  = false;

        
    }
    else if(dream == true)
    {
            // Dream Sequentially
            ShadowingDreamStateB();
            
    }
    lastPresentState = openCV.isSomeoneThere();
    ofDisableBlendMode(); 
}
//--------------------------------------------------------------
//* Production BASIC A - Walking under light triggers the most recent recording
//* Then playback the new recorded buffer
//* Then playback the previous buffers sequentially
//* If no users, Do the Dream State
//--------------------------------------------------------------
//--------------------------------------------------------------
void ofApp::ShadowingProductionModeB()
{
    // Default Dream State B
    
    // If under light but not in zone fade out dream state
    
    // If in trigger area Start recording and Playback most recent recording
    
    // When user exits playback recording
    
    // If no-one there Dream State B
}
//--------------------------------------------------------------
//* Production BASIC A - Walking under light triggers the most recent recording
//* Then playback the new recorded buffer
//* Then playback the previous buffers sequentially
//* If no users, Do the Dream State
//--------------------------------------------------------------
//--------------------------------------------------------------
void ofApp::ShadowingProductionTest()
{
    
    ofPushStyle();
    for (int i = 0; i < buffers.size(); i++)
    {
        buffers[i].draw(255);
    }
    
    ofPopStyle();
    
}
//--------------------------------------------------------------
//* BASIC B - Walking under light triggers the most recent recording
//* Then playback the new recorded buffer
//* Then playback the previous buffers sequentially
//* If no users, Do the Dream State
//--------------------------------------------------------------
//--------------------------------------------------------------
void ofApp::ShadowingModeA()
{
    // If someone is there play the last shadow
    if(openCV.isSomeoneThere())
    {
        modeString = "Shadowing Mode A";
        if (!buffers.empty())
        {
            buffers[0].start();
            buffers[0].draw(255);
        }
    }
    else if(!openCV.isSomeoneThere() && dream == false)
    {
        dream = true;
    }
    else if(dream == true)
    {
        // Dream Sequentially
        ShadowingDreamStateB();
    }
}
//--------------------------------------------------------------
//* BASIC B - Walking under light triggers the most recent recording
//* Then Dreams of Everybody
//--------------------------------------------------------------
//--------------------------------------------------------------
void ofApp::ShadowingModeB()
{
    if(openCV.isSomeoneThere())
    {
        modeString = "Shadowing Mode B";
        if (!buffers.empty())
        {
            buffers[0].start();
            buffers[0].draw(255);
        }
    }
    else if(!openCV.isSomeoneThere() && dream == false)
    {
        dream = true;
    }
    else if(dream == true)
    {
        //Dream of Everybody
        ShadowingDreamStateA();
    }
}
//--------------------------------------------------------------
//* Here are all the Buffer Playback Functions
//--------------------------------------------------------------
//--------------------------------------------------------------
void ofApp::ShadowingModeC()
{
    
    
}
//--------------------------------------------------------------
//* Here are all the Buffer Playback Functions
//--------------------------------------------------------------
//--------------------------------------------------------------
void ofApp::ShadowingModeD()
{
    if (!buffers.empty())
    {
        if (openCV.isSomeoneInTheLight() && !buffers[whichBufferAreWePlaying].isFinished())
        {
            ofDrawBitmapStringHighlight("Not finished yet", ofGetWidth()/2,ofGetHeight()/2);
        }
        else if (openCV.isSomeoneInTheLight() && buffers[whichBufferAreWePlaying].isFinished())
        {
            ofDrawBitmapStringHighlight("Finished Buffer", ofGetWidth()/2,ofGetHeight()/2);
        }
        else
        {
            
        }
    }
    else if(!openCV.isSomeoneThere() && dream == false)
    {
        dream = true;
    }
    else if(dream == true)
    {
        ShadowingDreamStateB();
    }
}
//--------------------------------------------------------------
//* Here are all the Buffer Playback Functions
//--------------------------------------------------------------
//--------------------------------------------------------------
void ofApp::ShadowingModeE()
{
    
    
}
//--------------------------------------------------------------
//* Here are all the Buffer Playback Functions
//--------------------------------------------------------------
//--------------------------------------------------------------
void ofApp::ShadowingModeF()
{
    
    
}
//--------------------------------------------------------------
//* Here are all the Buffer Playback Functions
//--------------------------------------------------------------
//--------------------------------------------------------------
void ofApp::ShadowingModeG()
{
    modeString = "Live Image and Buffer";
    if(openCV.isSomeoneThere())
    {
        if (!buffers.empty())
        {
            buffers[0].start();
        }
    }
    
    // Draw the first buffer in the Vector
    if (!buffers.empty())
    {
        openCV.drawLiveShadow();
        if (backColor.getBrightness() >=125)
        {
            ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
        }
        else if (backColor.getBrightness() <= 124)
        {
            ofEnableBlendMode(OF_BLENDMODE_ADD);
        }
        else
        {
            
        }
        buffers[0].draw(255);
        ofDisableBlendMode();
    }
}
//--------------------------------------------------------------
//* Here are all the Buffer Playback Functions
//--------------------------------------------------------------
//--------------------------------------------------------------
void ofApp::ShadowingModeH()
{
    ShadowingDefaultModeNoLoop();
}
//--------------------------------------------------------------
void ofApp::guiEvent(ofxUIEventArgs &e)
{
    if (e.getName() == "Progressive Background")
    {
        ofxUILabelToggle * toggle = (ofxUILabelToggle *) e.widget;
        bProgressiveLearning = toggle->getValue();
    }
    else if (e.getName() == "Do Calibration")
    {
        ofxUILabelToggle * toggle = (ofxUILabelToggle *) e.widget;
        doCalibration = toggle->getValue();
        openCV.setCalibration(toggle->getValue());
    }
    else if(e.getName() == "Use Mask")
    {
        ofxUILabelToggle * toggle = (ofxUILabelToggle *) e.widget;
        drawMask = toggle->getValue();
    }
    else if(e.getName() == "PLAYBACK_MODE")
    {
        ofxUINumberDialer * dial = (ofxUINumberDialer *) e.widget;
        //playbackMode = dial->getValue();
    }
    else if(e.getName() == "Mask_No")
    {
        ofxUINumberDialer * dial = (ofxUINumberDialer *) e.widget;
        whichMask = dial->getValue();
    }
    else if(e.getName() == "Learn Background")
    {
        ofxUILabelButton * button = (ofxUILabelButton *) e.widget;
        openCV.relearnBackground();
        //learnBackground = button->getValue();
    }
    else if(e.getName() == "PROGRESSIVE_RATE")
    {
        ofxUINumberDialer * dial = (ofxUINumberDialer *) e.widget;
        fProgressiveRate = dial->getValue();
    }
    else if(e.getName() == "BUFFER_NUMBER")
    {
        ofxUINumberDialer * dial = (ofxUINumberDialer *) e.widget;
        howManyBuffersToStore = dial->getValue();
    }
    else if(e.getName() == "TRACKING_BOUNDARY")
    {
        ofxUINumberDialer * dial = (ofxUINumberDialer *) e.widget;
        openCV.setTrackingBoundaries(dial->getValue(),dial->getValue());
    }
    else if(e.getName() == "BrightnessV")
    {
        ofxUINumberDialer * dial = (ofxUINumberDialer *) e.widget;
        brightness = dial->getValue();
    }
    else if(e.getName() == "ContrastV")
    {
        ofxUINumberDialer * dial = (ofxUINumberDialer *) e.widget;
        contrast = dial->getValue();
    }
    else if(e.getName() == "Fullscreen")
    {
        ofxUILabelToggle * toggle = (ofxUILabelToggle *) e.widget;
        ofSetFullscreen(toggle->getValue());
    }
    else if(e.getName() == "Erode")
    {
        ofxUILabelToggle * toggle = (ofxUILabelToggle *) e.widget;
        erode = toggle->getValue();
    }
    else if(e.getName() == "Dilate")
    {
        ofxUILabelToggle * toggle = (ofxUILabelToggle *) e.widget;
        dilate = toggle->getValue();
    }
    else if(e.getName() == "Show Buffers")
    {
        ofxUILabelToggle * toggle = (ofxUILabelToggle *) e.widget;
        showPreviousBuffers = toggle->getValue();
    }
    else if(e.getName() == "THRESHOLD")
    {
        ofxUINumberDialer * dial = (ofxUINumberDialer *) e.widget;
        threshold = dial->getValue();
    }
    else if(e.getName() == "MIN_BLOB_SIZE")
    {
        ofxUINumberDialer * dial = (ofxUINumberDialer *) e.widget;
        iMinBlobSize = dial->getValue();
    }
    else if(e.getName() == "MAX_BLOB_SIZE")
    {
        ofxUINumberDialer * dial = (ofxUINumberDialer *) e.widget;
        iMaxBlobSize = dial->getValue();
    }
    else if(e.getName() == "MAX_BLOB_NUM")
    {
        ofxUINumberDialer * dial = (ofxUINumberDialer *) e.widget;
        iMaxBlobNum = dial->getValue();
    }
    else if (e.getName() == "Mirror H")
    {
        ofxUILabelToggle * toggle = (ofxUILabelToggle *) e.widget;
        bMirrorH = toggle->getValue();
    }
    else if (e.getName() == "Mirror V")
    {
        ofxUILabelToggle * toggle = (ofxUILabelToggle *) e.widget;
        bMirrorV = toggle->getValue();
    }
    else if (e.getName() == "Use Approximation")
    {
        ofxUILabelToggle * toggle = (ofxUILabelToggle *) e.widget;
        bUseApprox = toggle->getValue();
    }
    else if (e.getName() == "Fill Holes")
    {
        ofxUILabelToggle * toggle = (ofxUILabelToggle *) e.widget;
        bFillHoles = toggle->getValue();
    }
    else if(e.getName() == "BLUR")
    {
        ofxUINumberDialer * dial = (ofxUINumberDialer *) e.widget;
        fBlur = dial->getValue();
    }
    else if (e.getName() == "Draw CV")
    {
        ofxUILabelToggle * toggle = (ofxUILabelToggle *) e.widget;
        drawCV = toggle->getValue();
    }
    else if (e.getName() == "Draw Live")
    {
        ofxUILabelToggle * toggle = (ofxUILabelToggle *) e.widget;
        drawLiveImage = toggle->getValue();
    }
    else if(e.widget->getName() == "Background_Color")
    {
        ofxUIImageSampler *sampler = (ofxUIImageSampler *) e.widget;
        backColor = sampler->getColor();
    }
    else if(e.widget->getName() == "Shadow_Color")
    {
        ofxUIImageSampler *sampler = (ofxUIImageSampler *) e.widget;
        shadowColor = sampler->getColor();
    }
    else if (e.getName() == "Show Data")
    {
        ofxUILabelToggle * toggle = (ofxUILabelToggle *) e.widget;
        canDrawData = toggle->getValue();
    }
    else if(e.getName() == "BLUR_PASS")
    {
        ofxUINumberDialer * dial = (ofxUINumberDialer *) e.widget;
        shader.setRotation(dial->getValue());

    }
    else if(e.getName() == "BLUR_RADIUS")
    {
        ofxUINumberDialer * dial = (ofxUINumberDialer *) e.widget;
        shader.setScale(dial->getValue());
    }
    else if (e.getName() == "Use Shader")
    {
        ofxUILabelToggle * toggle = (ofxUILabelToggle *) e.widget;
        useShader = toggle->getValue();
    }
}
//--------------------------------------------------------------
//* Debug Data
//--------------------------------------------------------------
//--------------------------------------------------------------
void ofApp::drawData()
{
    ofSetColor(255);
    string title = "Shadowing " + ofToString(ofGetTimestampString("%H:%M:%S  %d/%m/%Y"));
    stringstream debugData;
    debugData << title << endl;
    debugData << "Unit " << unitID << endl;
    debugData << "FrameRate: " << ofGetFrameRate() << endl;
    
    debugData << endl;
    debugData << "BUFFERS" << endl;
    debugData << "We have recorded: " << howmanyrecordings << " actions" << endl;
    debugData << "Current Recording Buffer Size " << imageCounter << endl;
    debugData << "Maximum Buffer Size: " << howManyBuffersToStore << endl;
    debugData << "Current Buffer Size: " << buffers.size() << endl;
    debugData << "Currently Playing Buffer: " << whichBufferAreWePlaying << endl;
    debugData << endl;
    
    if (!buffers.empty())
    {
        for (int i = 0; i < buffers.size(); i++)
        {
            debugData << "Buffer " << i+1 << " has " << buffers[i].getNumberOfFrames() << " Frames" << endl;
        }
    }
    debugData << endl;
    debugData << "OPENCV" << endl;
    debugData << "Currently Tracking " << openCV.getNumberOfBlobs() << " Number of Blobs " << endl;
    debugData << "Timer: " << CVstring << endl;
    debugData << endl;
    debugData << "FBO Current Brightness " << backColor.getBrightness() << endl;
    
    debugData << endl;
    debugData << "Current Mode: " << modeString << endl;

    debugData << endl;
    debugData << endl;
    
    debugData << "HTTP Status" << endl;
    debugData << responseStr << endl;
    debugData << requestStr << endl;
    debugData << endl;

    debugData << endl;
    debugData << latestGifPath << endl;
    
   
    ofDrawBitmapStringHighlight(debugData.str(), 5,ofGetHeight()/2);

}
