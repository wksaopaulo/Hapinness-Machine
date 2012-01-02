#pragma once

#include "ofMain.h"
#include "ofxCvMain.h"
#include "ofxSimpleGuiToo.h"

class happinessApp : public ofBaseApp{
    
    //Happiness triggers
    bool triggeredByMovement;
    bool triggeredBySound;
    bool triggeredBySocialNet;
    unsigned int lastHappiness;
    
    //Happiness maker
    ofArduino arduino;
    bool arduinoReady;
    void arduinoSetUp(const int & version);
    //Servo state
    int servoRotationTime; //When the servo opened the door
    int openedWindow; //Number of frames the window will remain open
    
    //Configuration helper
    ofxSimpleGuiToo gui;
    
    /*****
     Movement tracking
     *****/
    ofxCvGrayscaleImage currFrame;
    ofxCvGrayscaleImage lastFrame;
    ofxCvGrayscaleImage diffImage;
    ofVideoGrabber camera;
    
    int movementFrameTh;
    int movementTimespan;
    int movementMinFrames;
    //Number of happy frames in this timespan
    int happyFrames;
    
    /******
     Sound tracking
     *****/
    ofSoundStream soundStream;
    float smoothedVolume;
    vector<float> volHistory;
    
    float soundVolumeTh;
    int soundTimespan;
    int soundMinFrames;
    //Number of happy noises in this timespan
    int happyNoises;
    
    public:
		void setup();
		void update();
		void draw();

        void audioIn(float * input, int bufferSize, int nChannels); 
		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
};
