//--------------------------------------------------
// Name: Shadowing.cpp
// Project: Playable City 2014 Award
// Author: David Haylock
// Creation Date: 30-07-2014
// Copyright: (c) 2014 by Watershed Arts Trust Ltd.
//--------------------------------------------------
#include "ofApp.h"
//--------------------------------------------------------------
void ofApp::loadConfig()
{
    cout << "Loading Settings" << endl;
    if (XML.loadFile("/root/config.xml")) {
        _locationID = XML.getValue("CONFIG:LOCATIONID", "");
        _statusurl = XML.getValue("CONFIG:STATUSURL", "");
        _uploadFileURL = XML.getValue("CONFIG:UPLOADFILEURL", "");
        _secretKey = XML.getValue("CONFIG:SECRETKEY", "");
        cout << "Loaded Settings!" << endl;
    }
    else {
        cout << "Not Loaded Settings!" << endl;
    }
}

//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetWindowShape(ofGetScreenWidth(), ofGetScreenHeight());
    ofSetFrameRate(FRAMERATE*2);
    ofSetLogLevel(OF_LOG_VERBOSE);
    loadConfig();
    // Setup the Projector
    setupProjector();

    // Setup Variables
    setupVariables();

    // Setup Gif Encoder
    // setupGifEncoder();

    // Setup Directory Watcher
    // setupDirectoryWatcher();
#ifdef HAVE_WEB
    // Setup HTTP Stuff
    setupHTTP();
#else

#endif
    // Setup CV
    setupCV();

    cout << "after setup cv"<< endl;

    // Looks for masks inside of the Masks folder
    setupMasks();

	cout << "after setup masks"<< endl;

    // Setup the GUI
    //setupGUI();
    setupSimpleGUI();


    // Setup the Timer
    // setupTimers();
    dreamTimer = ofGetElapsedTimeMillis()+10000;
    statusTimer = ofGetElapsedTimeMillis()+30000;
    
    // Setup Shaders
    setupShader();
    playBackLatch = false;
    lastPresentState = false;


    ofSetVerticalSync(true);
    cout << "setup done"<< endl;

	//open usb relay
    ofSystem("sh /root/openusbrelay.sh");

    drawCamFull = false;
	
	//open logging file // and append
	outfile.open("activity.txt", std::ios::app);
    
    ofShowCursor();
    cursorDisplay = true;
}
//--------------------------------------------------------------
void ofApp::update()
{
    // Set Window Title
    // string title = "Shadowing: " + ofToString(ofGetTimestampString("%H:%M:%S  %d/%m/%Y"));
    // ofSetWindowTitle(title);

    //--------------------------------------------------------------
    // If we have too many buffers in the memory then release one
    if (buffers.size() > howManyBuffersToStore){
        buffers.pop_back();
    }

    // if (livebuffer.size() > 2) {
    //     livebuffer.pop_back();
    // }

    //--------------------------------------------------------------
    openCV.DsubtractionLoop(false,false);


    // If blob detected Start Recording
    if(openCV.isSomeoneThere() && imageCounter < MAX_BUFFER_SIZE){


    	// if (buffers.size() > 3){
    	// 	whichBufferAreWePlaying = 2;
    	// }else{
    	// 	whichBufferAreWePlaying = 0;
    	// }


        // canSaveGif = true;
        // activityTimer.stop();
        // doCVBackgroundTimer.stop();
        startRecording = true;
        hasBeenPushedFlag = false;
       
        dream = false;
        dreamTimer = ofGetElapsedTimeMillis() + 10000;
        // noneDream = false;
        // triggerDreamTimer = true;
    }else{

        bSwitch = true;
        startRecording = false;
        if (hasBeenPushedFlag == false){

            if (imageCounter >= MIN_BUFFER_SIZE){

		/* //Stops saving gifs to see if that keeps us online at all
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
		*/

                //buffers.push_front(b);
                //b.clear();
              	//Playing after a certain amount of frames have been recorded, instead of waiting for recording to end
		        //buffers[0].start();
		        //log activity to logfile
                
                outfile << "rec length," << imageCounter << ", timestamp;," << ofGetTimestampString("%m/%d,%H:%M") << endl;
                hasBeenPushedFlag = true;
                imageCounter = 0;
                // doCVBackgroundTimer.start(false);
            }else if(imageCounter < MIN_BUFFER_SIZE){
                buffers.pop_front();
                //b.clear();
                imageCounter = 0;
                hasBeenPushedFlag = true;
            }else{

            }
        }
        else
        {
            //Do nothing
        }
    }

    if(startRecording == true){
        // If new frame
        //if (openCV.newFrame())

       if(ofGetElapsedTimeMillis() - recTimer > 1000/FRAMERATE)
       {
            // Capture Gif Image every 5 frames
            if (ofGetFrameNum() % 5 == 0)
            {	
		//Turn off gif recording to see if that helps the internet situation
                //captureFrame();
            }
		//Start new recording
            if(imageCounter == 0){
		      buffers.push_front(b);
		      cout << "starting new videobuffer" << endl;
	    }
		//Playing recording before it has ended - put this frame number into settings
		if(imageCounter == delayFramesBeforePlayback){
			 buffers[0].start();
		}

		    // Capture the CV image
		    //videoBuffers.push_back(videoBuffer);
	        buffers[0].buffer.push_back(openCV.getRecordPixels());
		    //b.getNewImage(openCV.getRecordPixels());
                //blobPath.push_back(openCV.getBlobPath());
             imageCounter++;
             recTimer = ofGetElapsedTimeMillis();
	   }
    }

    if(!openCV.isSomeoneThere())
    {
//         if (canSaveGif == true)
//         {
//             #ifdef NUC
// //                gifEncoder.save(SAVE_PATH_NUC+ofGetTimestampString()+".gif");
//             #else
// //                gifEncoder.save(SAVE_PATH_MAC+ofGetTimestampString()+".gif");
//             #endif
//             howmanyrecordings++;
//             canSaveGif = false;
//         }
        stopLoop = true;
    }

    // if (!openCV.isSomeoneThere() && triggerDreamTimer == true){
    //     triggerDreamTimer = false;
        
    // }

    if(ofGetElapsedTimeMillis() > statusTimer){
        statusTimer = ofGetElapsedTimeMillis() + STATUS_FREQUENCY;
        sendStatus();
        cout << "status sent " << endl;

    }

    if(ofGetElapsedTimeMillis() > dreamTimer && dream == false){
        dream = true;
        cout << "dreaming started " << endl;
    }


    // Update the buffer progressors
    if (!buffers.empty())
    {
        for (int i = 0; i < buffers.size(); i++)
        {   
                buffers[i].update();
        }
    }

    // doCVBackgroundTimer.update();
    // statusTimer.update();
    // activityTimer.update();

    if (cursorDisplay == true)
    {
        ofShowCursor();
    }
    else
    {
        ofHideCursor();
    }


    //Draw to FBO 
    mainOut.begin();
        ofClear(backColor);

        ofPushStyle();
        ofSetColor(0);
        ofDrawBitmapString(ofGetTimestampString(),300,300);
        ofPopStyle();

        if (useShader){
            shader.begin();
            ofSetColor(255, 255);
            ofRect(0, 0, 320,240);
        }

        ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
        ofPushMatrix();
        ofTranslate(0,playbackOffsetY);
        if(!buffers.empty()){
            for (int i = 0; i < buffers.size(); i++){   
                //Buffers are only drawn if they are active, otherwise nothing happens during this call
                buffers[i].draw(255);
            }
        }
        ofPopMatrix();
        ofDisableBlendMode();

        //Trigger / reset playback of last buffer
        ShadowingProductionModeA();

        if (useShader)
        {
            shader.end();
            shader.draw();
        }
        
    mainOut.end();
    //End FBpO


}
//--------------------------------------------------------------
void ofApp::draw(){

    ofBackground(backColor);
    ofSetColor(255, 255, 255);
    mainOut.draw(0,0,ofGetWidth(),ofGetHeight());

    // As it implies does alpha layering and draws mask to blur the edges of the projection
    if (drawMask){
        ofEnableAlphaBlending();
        ofSetColor(255, 255);
        masks[whichMask].draw(-maskScale+maskCenterX,-maskScale+maskCenterY,ofGetWidth()+maskScale,ofGetHeight()+maskScale);
        ofDisableAlphaBlending();
    }

    drawMisc();
    openCV.drawGui();

	if(drawCamFull){
       ofPushMatrix();
	   ofTranslate(0,playbackOffsetY);
	   openCV.drawCameraFullScreen();
	   ofPopMatrix();
    }
}

//--------------------------------------------------------------
// Production BASIC A - Walking under light triggers the most recent recording
// Then playback the new recorded buffer
// Then playback the previous buffers sequentially
// If no users, Do the Dream State
//--------------------------------------------------------------
//--------------------------------------------------------------
void ofApp::ShadowingProductionModeA(){

    // if someone enters the light quickly
    if(openCV.isSomeoneThere() && openCV.isSomeoneThere() != lastPresentState && buffers.size() > 0 && buffers[1].isNearlyFinished())
    {
        playBackLatch = false;

        modeString = "Shadowing Basic Mode";
        buffers[1].reset();
        buffers[1].start();
    }

    //Just stepped out of the light
    else if(!openCV.isSomeoneThere() && dream == false && playBackLatch == false)
    {
        bSwitch = true;
        modeString = "Shadowing Basic Mode Stage 2";
        buffers[1].start();
        playBackLatch  = false;

    }
    else if(dream == true)
    {
            // Dream Sequentially
            ShadowingDreamStateB();

    }

    lastPresentState = openCV.isSomeoneThere();
    // ofDisableBlendMode();
}

//--------------------------------------------------------------
// Dream state - Play the buffers Sequentially
//--------------------------------------------------------------
void ofApp::ShadowingDreamStateB()
{
    modeString = "Dreaming Sequentially";
    // Check if the buffers are live. Start and Draw the first element in the vector.
    // When the buffer has finished playing the iterate to the next buffer
    if (!buffers.empty())
    {
        buffers[whichBufferAreWePlaying].start();
        
        // // This will change the Blend Modes according to the brightness of FBO
        // if (backColor.getBrightness() >=125)
        // {
        //     ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
        // }
        // else if (backColor.getBrightness() <= 124)
        // {
        //     ofEnableBlendMode(OF_BLENDMODE_ADD);
        // }
        // else
        // {
        //     // Nothing
        // }
        // //buffers[whichBufferAreWePlaying].draw(255);
        // ofDisableBlendMode();

        if (buffers.size() > 2)
        {
            if (buffers[whichBufferAreWePlaying].isFinished() && randomWaitLatch == false)
            {
                // randomWaitTimer = ofGetElapsedTimeMillis() + ofRandom(1000,4000);
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
void ofApp::keyPressed(int key)
{
    switch(key)
    {
        case '1':
	    imagingMode = 0;
	    break;
	case '2':
	    imagingMode = 1;
	    break;
	case '3':
	    playbackMode = 3;
	    break;
	case 'm':
            gui->toggleVisible();
	    drawCV = !drawCV;
        cursorDisplay = !cursorDisplay;

            // ((ofxUILabelToggle *) gui->getWidget("Draw CV"))->setValue(drawCV);
            break;
        // case OF_KEY_UP:
        //     projector.up();
        //     break;
        // case OF_KEY_DOWN:
        //     projector.down();
        //     break;
        // case OF_KEY_LEFT:
        //     projector.left();
        //     break;
        // case OF_KEY_RIGHT:
        //     projector.right();
        //     break;
        case 'i':
            projector.projectorOn();
            break;
        case 'o':
            projector.projectorOff();
            break;
        //case 'c':
        //    cursorDisplay = !cursorDisplay; // NULL on the ubuntu system
        //    break;
        case 'd':
            canDrawData = !canDrawData;
            ((ofxUILabelToggle *) gui->getWidget("Show Data"))->setValue(canDrawData);
            break;
        case 'v':
            openCV.toggleGui();
	        drawCV = !drawCV;
            ((ofxUILabelToggle *) gui->getWidget("Draw CV"))->setValue(drawCV);
            break;
        case 'c':
            	openCV.toggleGui();
		break;
	    case 'f':
		      drawCamFull = !drawCamFull;
		break;
	    case 'b':
            showPreviousBuffers = !showPreviousBuffers;
            // ((ofxUILabelToggle *) gui->getWidget("Show Buffers"))->setValue(showPreviousBuffers);
            break;
	    case 'z':
	       // playTiger();
	    break;

        case 't':
#ifdef HAVE_WEB      // Send the Gif to the Server
            ofxHttpForm form;
            form.action = _statusurl;
            form.method = OFX_HTTP_POST;
            form.addFormField("secret", _secretKey);
            form.addFormField("location", _locationID);
            form.addFormField("status", "TESTEVENT");
            form.addFormField("numberofrecordings", ofToString(howmanyrecordings));
            form.addFormField("submit","1");
            httpUtils.addForm(form);
            break;
         #else
            break;
         #endif

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
// //--------------------------------------------------------------
// void ofApp::newResponse(ofxHttpResponse & response)
// {
// 	responseStr = ofToString(response.status) + ":" + (string)response.responseBody;
// 	cout << responseStr << endl;
// }
//--------------------------------------------------------------
// void ofApp::onDirectoryWatcherItemAdded(const DirectoryWatcherManager::DirectoryEvent& evt)
// {

// #ifndef NUC
//     if (ofIsStringInString(evt.item.path(),".gif") && evt.item.getSize() > 5000)
//     {
//         cout << "Uploading File: " <<  latestGifPath << " Size: " << evt.item.getSize() <<" bytes " << endl;
//         latestGifPath = evt.item.path();

//         // Send the Gif to the Server
//         ofxHttpForm form;
//         form.action = _uploadFileURL;
//         form.method = OFX_HTTP_POST;
//         form.addFormField("secret", _secretKey);
//         form.addFormField("location", _locationID);
//         form.addFormField("timestamp",ofGetTimestampString());
//         form.addFormField("currentmode", modeString);
//         form.addFile("file",latestGifPath);
//         form.addFormField("submit","1");
//         httpUtils.addForm(form);
//     }
// #endif

// }
// //--------------------------------------------------------------------------------------------------
// void ofApp::onDirectoryWatcherItemRemoved(const DirectoryWatcherManager::DirectoryEvent& evt){ }
// //--------------------------------------------------------------------------------------------------
// void ofApp::onDirectoryWatcherItemModified(const DirectoryWatcherManager::DirectoryEvent& evt){ }
// //--------------------------------------------------------------------------------------------------
// void ofApp::onDirectoryWatcherItemMovedFrom(const DirectoryWatcherManager::DirectoryEvent& evt){ }
// //--------------------------------------------------------------------------------------------------
// void ofApp::onDirectoryWatcherItemMovedTo(const DirectoryWatcherManager::DirectoryEvent& evt)
// {

// #ifdef HAVE_WEB
// #ifdef NUC
//     if (ofIsStringInString(evt.item.path(),".gif") && evt.item.getSize() > 5000)
//     {
//         latestGifPath = evt.item.path();
//         cout << "File: " <<  latestGifPath << " Size : " <<  evt.item.getSize() << endl;

//         // Send the Gif to the Server
//         ofxHttpForm form;
//         form.action = _uploadFileURL;
//         form.method = OFX_HTTP_POST;
//         form.addFormField("secret", _secretKey);
//         form.addFormField("location", _locationID);
//         form.addFormField("timestamp",ofGetTimestampString());
//         form.addFormField("currentmode", modeString);
//         form.addFile("file",latestGifPath);
//         form.addFormField("submit","1");
//         httpUtils.addForm(form);
//     }
// #endif
// #endif
// }
// //--------------------------------------------------------------------------------------------------
// void ofApp::onDirectoryWatcherError(const Poco::Exception& exc){ }
//--------------------------------------------------------------
void ofApp::exit()
{
    // gifWatcher.unregisterAllEvents(this);

    ofSystem("sh /root/closeusbrelay.sh");
    
    // As it says
    // cleanGifFolder();
    #ifdef HAVE_WEB
    // Send the Gif to the Server
    // ofxHttpForm form;
    // form.action = _statusurl;
    // form.method = OFX_HTTP_POST;
    // form.addFormField("secret", _secretKey);
    // form.addFormField("location", _locationID);
    // form.addFormField("status", "STOPPED");
    // form.addFormField("numberofrecordings", ofToString(howmanyrecordings));
    // form.addFormField("submit","1");
    // httpUtils.addForm(form);
#endif
    cout << "Releasing Camera" << endl;
    openCV.releaseCamera();
    ofSleepMillis(1000);
    cout << "Released Camera" << endl;
#ifdef HAVE_WEB
    // httpUtils.stop();
	cout << "Stopped Web" << endl;
#endif
    openCV.exit();
	cout << "Exited Gui" << endl;
    gui->saveSettings("GUI/Settings.xml");
	delete gui;

    cout << "Saved Gui" << endl;

    #ifndef DEBUG
        projector.projectorOff();
    	cout << "Projector Off" << endl;
    #endif

	cout << "Finished Exit" << endl;

}
//--------------------------------------------------------------
// Other Stuff
//--------------------------------------------------------------
//--------------------------------------------------------------
// void ofApp::onGifSaved(string &fileName)
// {
//     // When the encoding process begins reset gifEncoder for next use
//     gifEncoder.reset();
// }
// //--------------------------------------------------------------
// // Other Stuff
// //--------------------------------------------------------------
// //--------------------------------------------------------------
// void ofApp::captureFrame()
// {
//     // Yep addFrame to the gifEncoder
//     gifEncoder.addFrame(openCV.getRecordPixels().getPixels(),CAM_WIDTH,CAM_HEIGHT,openCV.getRecordPixels().getBitsPerPixel(),0.1f);
// }
//--------------------------------------------------------------
//Clear the gifFolder at the end of the cycle
//--------------------------------------------------------------
//--------------------------------------------------------------
// void ofApp::cleanGifFolder()
// {
//     #ifdef NUC
//         ofSystem("rm -rfv /root/of_v0.8.3_linux64_release/apps/myApps/ShadowingApp/bin/data/gifs/*");
//     #else
//         ofSystem("rm -rfv /Users/davidhaylock/Desktop/of_v0.8.3_osx_release/apps/Shadowing/ShadowingStage0_5/bin/data/gifs/*");
//     #endif
// }
//--------------------------------------------------------------
//-------------------------------------------------------------
// SETUP ROUTINES
//-------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
// Setup OpenCV
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
// Setup Variables
//--------------------------------------------------------------
void ofApp::setupVariables()
{
    ofSetLogLevel(OF_LOG_WARNING);
    imageCounter = 0;
    playCounter = 0;
    dream = false;
    // triggerDreamTimer = false;
    // progress = 0;
    playbackMode = 0;
    howmanyrecordings = 0;
    whichBufferAreWePlaying = 0;
    imagingMode = 0;
    hasBeenPushedFlag = true;
    learnBackground = true;
    // canSaveGif = false;
    stopLoop = false;
    bSwitch = false;
    firstLearn = true;
    // noneDream == false;
    drawCV = true;
	recTimer = ofGetElapsedTimeMillis();
}
//--------------------------------------------------------------
// Setup Directory Watcher
//--------------------------------------------------------------
void ofApp::setupDirectoryWatcher()
{
// #ifdef NUC
//     // Add listener
//     gifWatcher.registerAllEvents(this);

//     // Folder to Watch
//     std::string gifFolderToWatch = SAVE_PATH_NUC;

//     // Do not list the existing files as there will be none
//     bool listExistingItemsOnStart = false;

//     gifWatcher.addPath(gifFolderToWatch, listExistingItemsOnStart, &fileFilter);

//     latestGifPath = "";
// #else
//     gifWatcher.registerAllEvents(this);

//     std::string gifFolderToWatch = SAVE_PATH_MAC;

//     bool listExistingItemsOnStart = false;

//     gifWatcher.addPath(gifFolderToWatch, listExistingItemsOnStart, &fileFilter);

//     latestGifPath = "";

// #endif
}
//--------------------------------------------------------------
// Setup HTTP Utils
//--------------------------------------------------------------
// void ofApp::setupHTTP()
// {
//     // Setup HTTP POST Unit
//     ofAddListener(httpUtils.newResponseEvent,this,&ofApp::newResponse);
// 	httpUtils.start();

//     // Send Status
//     ofxHttpForm form;
//     form.action = _statusurl;
//     form.method = OFX_HTTP_POST;
//     form.addFormField("secret", _secretKey);
//     form.addFormField("location", _locationID);
//     form.addFormField("status", "STARTED");
//     form.addFormField("numberofrecordings", ofToString(howmanyrecordings));
//     form.addFormField("submit","1");
//     httpUtils.addForm(form);
// }
//--------------------------------------------------------------
// Setup Gif Encoder
//--------------------------------------------------------------
// void ofApp::setupGifEncoder()
// {
//     gifEncoder.setup(CAM_WIDTH, CAM_HEIGHT, .25, 256);
//     ofAddListener(ofxGifEncoder::OFX_GIF_SAVE_FINISHED, this, &ofApp::onGifSaved);
// }
//--------------------------------------------------------------
// Setup Shader
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
// Setup Projector
//--------------------------------------------------------------
void ofApp::setupProjector()
{
    // Connect to the projector
    #ifndef DEBUG
        projector.openConnection("/dev/ttyUSB0",9600);
	    cout << "after opening projector"<< endl;
        // projector.turnOffCamera();
        // Turn the projector On
        projector.projectorOn();
	#endif
}
//--------------------------------------------------------------
//  Setup Masks
//--------------------------------------------------------------
void ofApp::setupMasks()
{
    
	// Look inside of the Masks folder
    cout << "before opening directory" << endl;

	ofDirectory maskDirectory;
    int nFiles = maskDirectory.listDir("Masks");

    // Not sure why I've sorted them?
    maskDirectory.sort();
	cout << "after opening director"<< endl;
    if(nFiles)
    {
        ofLog(OF_LOG_NOTICE, "Found Mask Folder");
        for(int i = 0; i< maskDirectory.numFiles(); i++)
        {
            string filePath = maskDirectory.getPath(i);
            cout << filePath << endl;
		masks[i].loadImage(filePath);
        }
    }
    else
    {
        ofLog(OF_LOG_ERROR, "Can't Find Mask Folder");
    }

	cout << "done loading masks" <<endl;
}
//--------------------------------------------------------------
// Setup Timers
//--------------------------------------------------------------
// void ofApp::setupTimers(){

    //Add to UI 
    //dreamTimer.setup(10000); //set back to 30 secs
	//ofAddListener(dreamTimer.TIMER_STARTED, this, &ofApp::dreamTimerStarted);
    //ofAddListener(dreamTimer.TIMER_COMPLETE, this, &ofApp::dreamTimerComplete);
    // dreamTimer = ofGetElapsedTimeMillis();

    // statusTimer = ofGetElapsedTimeMillis();
    //Send Status
    // statusTimer.setup(STATUS_TIMER); // Every 2 minutes 1000 millis * 60 seconds * 2
    // ofAddListener(statusTimer.TIMER_STARTED, this, &ofApp::statusTimerStarted);
    // ofAddListener(statusTimer.TIMER_COMPLETE, this, &ofApp::statusTimerComplete);
    // statusTimer.start();

// }
//--------------------------------------------------------------
// Setup GUI
//--------------------------------------------------------------
//--------------------------------------------------------------

void ofApp::setupSimpleGUI()
{
    gui = new ofxUICanvas(ofGetWidth()/2,0,600,600);
    gui->setColorBack(ofColor::black);

    gui->addWidgetDown(new ofxUILabel("Basic Settings", OFX_UI_FONT_MEDIUM));
    gui->addSpacer(255,1);
    gui->addSpacer(0,10);
  
    // gui->addWidgetDown(new ofxUILabel("Imaging Threshold", OFX_UI_FONT_MEDIUM));
    // gui->addWidgetRight(new ofxUINumberDialer(0, 255, 80, 0, "IMAGE_THRESHOLD", OFX_UI_FONT_MEDIUM));
    
    gui->addWidgetDown(new ofxUILabel("Playback Offset Y", OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUINumberDialer(-1000, 1000, 80, 0, "PLAYBACK_OFFSET_Y", OFX_UI_FONT_MEDIUM));
    
    gui->addWidgetDown(new ofxUILabel("Mask Scaler", OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUINumberDialer(0, 1000, 1, 0, "MASK_SCALE", OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabel("Mask Center X ", OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUINumberDialer(-500, 500, 1, 0, "MASK_CENTER_X", OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabel("Mask Center Y", OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUINumberDialer(-500, 500, 1, 0, "MASK_CENTER_Y", OFX_UI_FONT_MEDIUM));

    gui->addWidgetDown(new ofxUILabel("Frame delay before Playback", OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUINumberDialer(0, 400, 30, 0, "DELAY_BEFORE_PLAYBACK", OFX_UI_FONT_MEDIUM));
	gui->addSpacer(0,30);
 //    gui->addWidgetDown(new ofxUILabel("Advanced Settings", OFX_UI_FONT_MEDIUM));
 //    gui->addSpacer(255,1);  
	// gui->addSpacer(0,10);  

    // gui->addWidgetDown(new ofxUILabel("Blur", OFX_UI_FONT_MEDIUM));
    // gui->addWidgetRight(new ofxUINumberDialer(0, 100, 1, 1, "BLUR", OFX_UI_FONT_MEDIUM));

    gui->addWidgetDown(new ofxUILabel("Tracking Boundaries", OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUINumberDialer(0, CAM_HEIGHT/2,5, 0, "TRACKING_BOUNDARY", OFX_UI_FONT_MEDIUM));
 
    // gui->addWidgetDown(new ofxUILabel("Movement Threshold", OFX_UI_FONT_MEDIUM));
    // gui->addWidgetRight(new ofxUINumberDialer(0, 255, 80, 0, "MOVE_THRESHOLD", OFX_UI_FONT_MEDIUM));


    // gui->addWidgetDown(new ofxUILabel("Brightness", OFX_UI_FONT_MEDIUM));
    // gui->addWidgetRight(new ofxUINumberDialer(0, 100, 1, 1, "BrightnessV", OFX_UI_FONT_MEDIUM));
    // gui->addWidgetDown(new ofxUILabel("Contrast", OFX_UI_FONT_MEDIUM));
    // gui->addWidgetRight(new ofxUINumberDialer(0, 100, 1, 1, "ContrastV", OFX_UI_FONT_MEDIUM));


//    gui->addWidgetDown(new ofxUILabelToggle("Fullscreen",true,255,30,OFX_UI_FONT_MEDIUM));
    // gui->addWidgetDown(new ofxUILabelToggle("Set Warp",false,255,30,OFX_UI_FONT_MEDIUM));
    // gui->addWidgetDown(new ofxUILabelToggle("Draw CV",false,255,30,OFX_UI_FONT_MEDIUM));
    // gui->addWidgetDown(new ofxUILabelToggle("Show Buffers",false,255,30,OFX_UI_FONT_MEDIUM));
    // gui->addWidgetDown(new ofxUILabelToggle("Show Data",false,255,30,OFX_UI_FONT_MEDIUM));
    // gui->addWidgetDown(new ofxUILabel("Imaging Mode", OFX_UI_FONT_MEDIUM));
    // gui->addWidgetRight(new ofxUINumberDialer(0, 12,1, 0, "IMAGING_MODE", OFX_UI_FONT_MEDIUM));
    // gui->addWidgetDown(new ofxUILabel("Number of Buffers", OFX_UI_FONT_MEDIUM));
    // gui->addWidgetRight(new ofxUINumberDialer(0, 15,5, 0, "BUFFER_NUMBER", OFX_UI_FONT_MEDIUM));
    // gui->addWidgetDown(new ofxUILabelToggle("Use Mask",true,255,30,OFX_UI_FONT_MEDIUM));
    // gui->addWidgetDown(new ofxUILabel("Mask Number", OFX_UI_FONT_MEDIUM));
    // gui->addWidgetRight(new ofxUINumberDialer(0, 5, 1, 0, "Mask_No", OFX_UI_FONT_MEDIUM));
  
    // gui->addWidgetDown(new ofxUILabel("Min Blob Size", OFX_UI_FONT_MEDIUM));
    // gui->addWidgetRight(new ofxUINumberDialer(0, (CAM_WIDTH*CAM_HEIGHT)/3, 20, 1, "MIN_BLOB_SIZE", OFX_UI_FONT_MEDIUM));
    // gui->addWidgetDown(new ofxUILabel("Max Blob Size", OFX_UI_FONT_MEDIUM));
    // gui->addWidgetRight(new ofxUINumberDialer(0, (CAM_WIDTH*CAM_HEIGHT), (CAM_WIDTH*CAM_HEIGHT)/3, 1, "MAX_BLOB_SIZE", OFX_UI_FONT_MEDIUM));
    // gui->addWidgetDown(new ofxUILabel("Max Num Blob", OFX_UI_FONT_MEDIUM));
    // gui->addWidgetRight(new ofxUINumberDialer(0, 10, 2, 0, "MAX_BLOB_NUM", OFX_UI_FONT_MEDIUM));
    // gui->addWidgetDown(new ofxUILabelToggle("Fill Holes",false,255/2,30,OFX_UI_FONT_MEDIUM));
    // gui->addWidgetRight(new ofxUILabelToggle("Use Approximation",false,255/2,30,OFX_UI_FONT_MEDIUM));
 
    // gui->addWidgetDown(new ofxUILabel("Gauss Blur", OFX_UI_FONT_MEDIUM));
    // gui->addWidgetRight(new ofxUINumberDialer(0, 100, 1, 1, "GAUSS_BLUR", OFX_UI_FONT_MEDIUM));

    // gui->addWidgetDown(new ofxUILabel("Median Blur", OFX_UI_FONT_MEDIUM));
    // gui->addWidgetRight(new ofxUINumberDialer(0, 100, 1, 1, "MEDIAN_BLUR", OFX_UI_FONT_MEDIUM));

 
    // gui->addWidgetDown(new ofxUILabelToggle("Erode",false,255/2,30,OFX_UI_FONT_MEDIUM));
    // gui->addWidgetRight(new ofxUILabelToggle("Dilate",false,255/2,30,OFX_UI_FONT_MEDIUM));
    // gui->addWidgetDown(new ofxUILabelToggle("Progressive Background",false,255,30,OFX_UI_FONT_MEDIUM));
    // gui->addWidgetDown(new ofxUILabel("Progression Rate", OFX_UI_FONT_MEDIUM));
    // gui->addWidgetRight(new ofxUINumberDialer(0.00f, 1.00f, 0.01f, 4, "PROGRESSIVE_RATE", OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabelToggle("Use Shader",false,255/2,30,OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabel("Blur Radius", OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUINumberDialer(0, 100, 1, 2, "BLUR_RADIUS", OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabel("Blur Pass", OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUINumberDialer(0, 100, 1, 2, "BLUR_PASS", OFX_UI_FONT_MEDIUM));
    //gui->addWidgetDown(new ofxUINumberDialer(0,10,3,1,"TIGER_PROBABILITY", OFX_UI_FONT_MEDIUM));
    gui->autoSizeToFitWidgets();
    ofAddListener(gui->newGUIEvent,this, &ofApp::guiEvent);

    gui->loadSettings("GUI/Settings.xml");
    gui->setVisible(false);

}

// --------------------------------------------------------------
void ofApp::sendStatus() //int &args
{
    // Send Status to the Server
    // ofxHttpForm form;
    // form.action = _statusurl;
    // form.method = OFX_HTTP_POST;
    // form.addFormField("secret", _secretKey);
    // form.addFormField("location", _locationID);
    // form.addFormField("status", "ON");
    // form.addFormField("numberofrecordings", ofToString(howmanyrecordings));
    // form.addFormField("submit","1");
    // httpUtils.addForm(form);

    // // Pulse the Projector
    // projector.projectorOn();

    // //Make sure relay is open!
    // //open usb relay
    // ofSystem("sh /root/openusbrelay.sh");

}
//--------------------------------------------------------------
// void ofApp::statusTimerStarted() //int &args
// {

// }

//--------------------------------------------------------------
// void ofApp::dreamTimerComplete()
// {
//     cout << "Timer Complete - dreaming starts now" << endl;
// 	// noneDream = false;
// 	dream = true;
// 	//tiger1.play();
// }
//--------------------------------------------------------------
// void ofApp::dreamTimerStarted() //int &args
// {
//     cout << "triggered Timer" << endl;
// }
//--------------------------------------------------------------
void ofApp::drawMisc()
{
    // Draw all the CV Stuff
    // ofPushStyle();
    // if(drawCV == true)
    // {
    //     openCV.draw();
    // }
    // ofPopStyle();

    // // Show the previous Buffers
    // ofPushStyle();
    //  if (showPreviousBuffers)
    // {
    //     if (!buffers.empty())
    //     {
    //         for (int i = 0; i < buffers.size(); i++)
    //         {
    //             // Draw the Mini Buffers
    //             // buffers[i].drawMini(640, 0+(i*240/4));
    //         }
    //     }
    //     if (!livebuffer.empty())
    //     {
    //         for (int i = 0; i < livebuffer.size(); i++)
    //         {
    //             // Draw the Mini Buffers
    //             // livebuffer[i].drawMini(640+80, 0+(i*240/4));
    //         }
    //     }
    // }
    // ofPopStyle();

    // // Debug for information about the buffers, openCV HTTP Posts
    // if (canDrawData == true)
    // {
    //     drawData();
    // }
}



// //--------------------------------------------------------------
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
    else if(e.getName() == "IMAGING_MODE")
    {
        ofxUINumberDialer * dial = (ofxUINumberDialer *) e.widget;
        imagingMode = dial->getValue();
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
    else if(e.getName() == "IMAGE_THRESHOLD")
    {
        ofxUINumberDialer * dial = (ofxUINumberDialer *) e.widget;
        threshold = dial->getValue();
    }
     else if(e.getName() == "PLAYBACK_OFFSET_Y")
    {
        ofxUINumberDialer * dial = (ofxUINumberDialer *) e.widget;
        playbackOffsetY = dial->getValue();
    }
	 else if(e.getName() == "MASK_SCALE")
    {
        ofxUINumberDialer * dial = (ofxUINumberDialer *) e.widget;
        maskScale = dial->getValue();
    }
  else if(e.getName() == "MASK_CENTER_X")
    {
        ofxUINumberDialer * dial = (ofxUINumberDialer *) e.widget;
        maskCenterX = dial->getValue();
    }
  else if(e.getName() == "MASK_CENTER_Y")
    {
        ofxUINumberDialer * dial = (ofxUINumberDialer *) e.widget;
        maskCenterY = dial->getValue();
    }

    else if(e.getName() == "DELAY_BEFORE_PLAYBACK")
    {
        ofxUINumberDialer * dial = (ofxUINumberDialer *) e.widget;
        delayFramesBeforePlayback = dial->getValue();
    }
     else if(e.getName() == "MOVE_THRESHOLD")
    {
        ofxUINumberDialer * dial = (ofxUINumberDialer *) e.widget;
        moveThreshold = dial->getValue();
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
    else if(e.getName() == "GAUSS_BLUR")
    {
        ofxUINumberDialer * dial = (ofxUINumberDialer *) e.widget;
        gaussBlur = dial->getValue();
    }
    else if(e.getName() == "MEDIAN_BLUR")
    {
        ofxUINumberDialer * dial = (ofxUINumberDialer *) e.widget;
        medianBlur = dial->getValue();
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
   // else if(e.widget->getName() == "Background_Color")
   // {
   //     ofxUIImageSampler *sampler = (ofxUIImageSampler *) e.widget;
   //     backColor = sampler->getColor();
   // }
   // else if(e.widget->getName() == "Shadow_Color")
   // {
   //     ofxUIImageSampler *sampler = (ofxUIImageSampler *) e.widget;
   //     shadowColor = sampler->getColor();
   // }

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
//    else if (e.getName() == "DREAM_WAIT_TIME")
//     {
//	ofxUINumberDialer * dial = (ofxUINumberDialer *) e.widget;
//	dreamWaitTime =  dial->getValue();
//     }
    // else if (e.getName() == "TIGER_PROBABILITY")
    //  {
    //     ofxUINumberDialer * dial = (ofxUINumberDialer *) e.widget;
    //     tigerProbability =  dial->getValue();
    //  }

}

//--------------------------------------------------------------
// Debug Data
//--------------------------------------------------------------
//--------------------------------------------------------------
void ofApp::drawData()
{
    ofSetColor(255);
    string title = "Shadowing " + ofToString(ofGetTimestampString("%H:%M:%S  %d/%m/%Y"));
    stringstream debugData;
    debugData << title << endl;
    debugData << "Unit " << _locationID << endl;
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
    // debugData << "Currently Tracking " << openCV.getNumberOfBlobs() << " Number of Blobs " << endl;
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


    ofDrawBitmapStringHighlight(debugData.str(), ofGetWidth()/2,0);

}


//Extra modes:

// void ofApp::ShadowingDreamStateA()
// {
//     modeString = "Dreaming About Everybody";
//     if (!buffers.empty())
//     {
//         for (int i = 0; i < buffers.size(); i++)
//         {
//             buffers[i].start();
//         }
//     }

//     if (!buffers.empty())
//     {
//         for (int i = 0; i < buffers.size(); i++)
//         {
//             if (buffers[i].isFinished())
//             {
//                 buffers[i].reset();
//             }
//         }
//     }

//     if (!buffers.empty())
//     {
//         for (int i = 0; i < buffers.size(); i++)
//         {
//             // This will change the Blend Modes according to the brightness of FBO
//             if (backColor.getBrightness() >=125)
//             {
//                 ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
//             }
//             else if (backColor.getBrightness() <= 124)
//             {
//                 ofEnableBlendMode(OF_BLENDMODE_ADD);
//             }
//             else
//             {
//                 // Nothing
//             }

//             buffers[i].draw(255);
//             ofDisableBlendMode();
//         }
//     }
// }
