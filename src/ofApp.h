#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxCvPiCam.h"
#include "ofxOsc.h"
#include "ofxXmlSettings.h"
#include "ofxHTTP.h"
#include "ofxJSONElement.h"
#include "ofxCrypto.h"
#include "IPVideoGrabber.h"

#define NUM_MESSAGES 30 // how many past ws messages we want to keep

class ofApp : public ofBaseApp {

    public:	
		void setup();
		void update();
		void draw();
			
		int width, height, appFramerate, camFramerate;
		int thumbWidth, thumbHeight;
		string sessionId, hostName; 
		string oscHost;
		int oscPort, streamPort, wsPort, postPort;

		bool debug; // draw to local screen, default true

		ofFile file;
		ofxXmlSettings settings;

		int rpiCamVersion; // 0 for not an RPi cam, 1, 2, or 3
		string lastPhotoTakenName;
		int stillCompression;
		int timestamp;
		
		void createResultHtml(string fileName);
		void takePhoto();
		void streamPhoto();
		vector<string> photoFiles;

		ofxHTTP::SimpleIPVideoServer streamServer;

		ofxHTTP::SimplePostServer postServer;
		void onHTTPPostEvent(ofxHTTP::PostEventArgs& evt);
		void onHTTPFormEvent(ofxHTTP::PostFormEventArgs& evt);
		void onHTTPUploadEvent(ofxHTTP::PostUploadEventArgs& evt);
		    	       
	    ofxHTTP::SimpleWebSocketServer wsServer;  
		void onWebSocketOpenEvent(ofxHTTP::WebSocketEventArgs& evt);
		void onWebSocketCloseEvent(ofxHTTP::WebSocketCloseEventArgs& evt);
		void onWebSocketFrameReceivedEvent(ofxHTTP::WebSocketFrameEventArgs& evt);
		void onWebSocketFrameSentEvent(ofxHTTP::WebSocketFrameEventArgs& evt);
		void onWebSocketErrorEvent(ofxHTTP::WebSocketErrorEventArgs& evt);
	    
		// ~ ~ ~ ~ ~ ~ ~     

		bool sendOsc;  // send osc
		bool sendWs;  // send websockets
		bool sendHttp;  // serve web control panel
		bool sendMjpeg;  // send mjpeg stream	

		bool syncVideo;  // send video image over osc
		bool brightestPixel;  // send brightest pixel
		bool blobs;  // send blob tracking
		bool contours; // send contours

		ofBuffer videoBuffer;
		ofBuffer photoBuffer;
		ofBuffer contourColorBuffer;
		ofBuffer contourPointsBuffer;
	
		ofFbo planeFbo, screenFbo;
		ofPixels planePixels, screenPixels;
		
		ofImage piCamTarget, camUsbTarget, mjpegInTarget;

		bool usePiCam;
		bool useUsbCam;
		bool useMjpegIn;

		bool newFrameToProcess;

		ofxCvPiCam cam;
		ofVideoGrabber camUsb;
		ofx::Video::IPVideoGrabber ipGrabber;
		
		string mjpegUrl;
		int camUsbId;
		cv::Mat frame, frame2, frame2Processed;
				
		ofImage mainImage;
		ofImage mainImageThumbnail;
		int syncVideoQuality; // 5 best to 1 worst, default 3 medium
		bool videoColor;

		// for more camera settings, see:
		// https://github.com/orgicus/ofxCvPiCam/blob/master/example-ofxCvPiCam-allSettings/src/testApp.cpp

	    int camRotation;        
	    int camShutterSpeed; // 0 to 330000 in microseconds, default 0
	    int camSharpness; // -100 to 100, default 0
	    int camContrast; // -100 to 100, default 0
	    int camBrightness; // 0 to 100, default 50
		int camIso; // 100 to 800, default 300
		int camExposureCompensation; // -10 to 10, default 0;

		// 0 off, 1 auto, 2 night, 3 night preview, 4 backlight, 5 spotlight, 6 sports, 7, snow, 8 beach, 9 very long, 10 fixed fps, 11 antishake, 12 fireworks, 13 max
		int camExposureMode; // 0 to 13, default 0

		//string oscAddress;
		int thresholdValue; // default 127
		int thresholdKeyCounter;
		bool thresholdKeyFast;

		ofxOscSender sender;
		
		ofxCv::ContourFinder contourFinder;
		float contourThreshold;  // default 127
		float contourMinAreaRadius; // default 10
		float contourMaxAreaRadius; // default 150
		int contourSlices; // default 20
		float simplify;
		int smooth;
			
		ofxCv::TrackingColorMode trackingColorMode; // RGB, HSV, H, HS; default RGB

        ofShader shader;
        string shaderName;
        ofPlanePrimitive plane;
        int planeResX, planeResY;
		bool doWireframe;

		void grabberSetup(int _id, int _fps, int _width, int _height);

		int sourceCounter;

};
