#include "happinessApp.h"

#define CAM_WIDTH 320
#define CAM_HEIGHT 240

#define FRAMERATE 60
#define SERVO_PIN 9

#define SERVO_INITIAL_ANGLE 10

//Tmp image
ofxCvColorImage currFrameColor;

//--------------------------------------------------------------
void happinessApp::setup(){
    
    ofSetFrameRate(FRAMERATE);
    
    //Setup interface
    gui.setup();
    
    /****
     Movement setup
     ****/
    //Turn camera on
    camera.initGrabber(CAM_WIDTH, CAM_HEIGHT);
    //Allocate space for images
    currFrameColor.allocate(CAM_WIDTH, CAM_HEIGHT);
    currFrame.allocate(CAM_WIDTH, CAM_HEIGHT);
    lastFrame.allocate(CAM_WIDTH, CAM_HEIGHT);
    diffImage.allocate(CAM_WIDTH, CAM_HEIGHT);
    //
    gui.addTitle("Movement");
    gui.addSlider("Frame threshold", movementFrameTh, 0, CAM_WIDTH*CAM_HEIGHT*10);
    gui.addSlider("Seconds to wait 0..30min", movementTimespan, 0, 60*30);
    gui.addSlider("Happy frames threshold", movementMinFrames, 0, 2000);
    happyFrames=0;
    
    /*********
     Sound Setup
     ********/
    int bufferSize = 256;
    soundStream.setup(this, 0, 2, 44100, bufferSize, 4);
    smoothedVolume = happyNoises = 0;
    //
    gui.addTitle("Sound");
    gui.addSlider("Volume threshold", soundVolumeTh, 0, 0.1);
    gui.addSlider("Seconds to wait 0..30min", soundTimespan, 0, 60*30);
    gui.addSlider("Happy noises threshold", soundMinFrames, 0, 2000);
    
    /*********
     Hardware communication
     ********/
    arduinoReady = false;
    ofAddListener(arduino.EInitialized, this, &happinessApp::arduinoSetUp);
    arduino.connect("/dev/tty.usbserial-A700e0mh", 57600);
    servoRotationTime = 0;
    gui.addTitle("Servo");
    gui.addSlider("Servo closing time window", openedWindow, 0, 240);
    
    
    ///////////////////////////////////////////////////////////////////
    gui.loadFromXML();
    gui.show();
    
    //Reset variables
    triggeredByMovement = triggeredBySocialNet = triggeredBySound = false;
    lastHappiness = 0;
    
}
//--------------------------------------------------------------
void happinessApp::arduinoSetUp(const int & version) {
	
    
	// remove listener because we don't need it anymore
	ofRemoveListener(arduino.EInitialized, this, &happinessApp::arduinoSetUp);
    
    
	// this is where you setup all the pins and pin modes, etc
	//for (int i = 0; i < 13; i++){
	//	arduino.sendDigitalPinMode(i, ARD_INPUT);
	//}
    arduino.sendDigitalPinMode(SERVO_PIN, ARD_SERVO);
	arduino.sendDigitalPinMode(13, ARD_OUTPUT);
	arduino.sendAnalogPinReporting(0, ARD_ANALOG);	// AB: report data
	arduino.sendDigitalPinMode(11, ARD_PWM);		// on diecimelia: 11 pwm?
    
    //Reset servo
    arduino.sendServo(SERVO_PIN, SERVO_INITIAL_ANGLE);

	arduinoReady = true;
}

//--------------------------------------------------------------
int pixelSum;
void happinessApp::update(){
    
    /******
     Movement capture
     *****/
    camera.grabFrame();
    if (camera.isFrameNew()) {
        //Saving last frame for difference comparation
        lastFrame = currFrame;
        
        //update current frame
        currFrameColor.setFromPixels(camera.getPixels(), CAM_WIDTH, CAM_HEIGHT);
        currFrame = currFrameColor;
        
        //Calculate movement
        diffImage = lastFrame;
        diffImage -= currFrame;
        
        //Check if it's a happy frame
        pixelSum = 0;
        for (int i=0; i < CAM_WIDTH*CAM_HEIGHT; i++) {
            pixelSum += diffImage.getPixels()[i];
        }
        if (pixelSum > movementFrameTh)
            happyFrames++;
        
    }
    
    //Check if it's time calculate the total movement of the timespan (in seconds)
    if (ofGetFrameNum() % (movementTimespan*FRAMERATE) == 0) {
        if (happyFrames > movementMinFrames) {
            triggeredByMovement = true;
        }
        
        //Reset counter
        happyFrames = 0;
        lastHappiness = ofGetElapsedTimef();
    }
    
    /******
     Sound
     ******/
    
    //Check if it's time calculate happiness based on time
    if (ofGetFrameNum() % (soundTimespan*FRAMERATE) == 0) {
        if (happyNoises > soundMinFrames) {
            triggeredBySound = true;
        }
        
        //Reset counter
        happyNoises = 0;
        lastHappiness = ofGetElapsedTimef();
    }
    
    /******
     Hardware
     ******/
    arduino.update();
    if (arduinoReady) {
        //Are we happy?
        if(triggeredByMovement || triggeredBySocialNet || triggeredBySound){
            
            //reset values
            triggeredByMovement = triggeredBySocialNet = triggeredBySound = false;
            
            arduino.sendServo(SERVO_PIN, 180);
            servoRotationTime = ofGetFrameNum();
            cout << "sent 180" << endl;
        }
        if (servoRotationTime + openedWindow == ofGetFrameNum()){
            arduino.sendServo(SERVO_PIN, SERVO_INITIAL_ANGLE);
            cout << "sent SERVO_INITIAL_ANGLE" << endl;
        }
        
    }
        
}

//--------------------------------------------------------------
void happinessApp::draw(){
    
    gui.draw();
    
    /*****
     Movement
     *****/
    ofPushMatrix();
    ofPushStyle();
    ofTranslate(250, 100);
    currFrame.draw(0, 0);
    if(pixelSum > movementFrameTh) ofSetColor(255, 0, 0);
    diffImage.draw(CAM_WIDTH, 0);
    //Write status
    string movementStatus = ofToString(happyFrames) + " happy frames. Movement trigger: " + ofToString(triggeredByMovement);
    ofDrawBitmapString(movementStatus, 0, CAM_HEIGHT + 10);
    ofPopStyle();
    ofPopMatrix();
    
    /*****
     Sound
     *****/
    ofPushMatrix();
    ofPushStyle();
    
    ofTranslate(250, 430);
    ofBeginShape();
    ofVertex(0, 0);
    for (int i=0; i<volHistory.size(); i++) {
        ofVertex(i, -volHistory[i]*1000);
    }
    ofVertex(volHistory.size(), 0);
    ofEndShape();
    //Write status
    string soundStatus = ofToString(happyNoises) + " happy noises. Sound trigger: " + ofToString(triggeredBySound);
    ofDrawBitmapString(soundStatus, 0, 15);
    //Draw threshold line
    ofSetColor(255, 0, 0);
    ofLine(0, -soundVolumeTh*1000, volHistory.size(), -soundVolumeTh*1000);
    
    ofPopStyle();
    ofPopMatrix();
    
    ofDrawBitmapString("Happy at " + ofToString(lastHappiness), 0, ofGetHeight()-30);
}


//--------------------------------------------------------------
void happinessApp::audioIn(float * input, int bufferSize, int nChannels) {	
	
    float curVol = 0.0;
	
	// samples are "interleaved"
	int numCounted = 0;	
    
	//lets go through each sample and calculate the root mean square which is a rough way to calculate volume	
	for (int i = 0; i < bufferSize; i++){
		float left      = input[i*2]*0.5;
		float right     = input[i*2+1]*0.5;
        
		curVol += left * left;
		curVol += right * right;
		numCounted+=2;
	}
	
	//this is how we get the mean of rms :) 
	curVol /= (float)numCounted;
	
	// this is how we get the root of rms :) 
	curVol = sqrt( curVol );
	
	smoothedVolume *= 0.93;
	smoothedVolume += 0.07 * curVol;
    
    volHistory.push_back(smoothedVolume);
    const float VOLUME_HISTORY_SIZE = 400;
    if(volHistory.size() > VOLUME_HISTORY_SIZE)
        volHistory.erase(volHistory.begin(), volHistory.begin()+1);
    
    if(smoothedVolume > soundVolumeTh)
        happyNoises++;
	
}

//--------------------------------------------------------------
void happinessApp::keyPressed(int key){

}

//--------------------------------------------------------------
void happinessApp::keyReleased(int key){

}

//--------------------------------------------------------------
void happinessApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void happinessApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void happinessApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void happinessApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void happinessApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void happinessApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void happinessApp::dragEvent(ofDragInfo dragInfo){ 

}