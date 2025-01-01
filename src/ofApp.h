#pragma once

#include "ofMain.h"
#include "ofxCv.h"
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
			
		int width, height, appFramerate;
		int thumbWidth, thumbHeight;
		string sessionId, hostName; 
		string oscHost;
		int oscPort, streamPort, wsPort, postPort, localStreamPort;

		bool debug; // draw to local screen, default true

		ofFile file;
		ofxXmlSettings settings;

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
		
		ofImage remoteIpImage, localIpImage;

		bool newFrameToProcess;

		ofx::Video::IPVideoGrabber remoteIpGrabber, localIpGrabber;
		bool useLocalIpGrabber;
		
		string mjpegUrl, localMjpegUrl;
		cv::Mat frame_first, frame, frameProcessed;
				
		ofImage gray;
		ofImage grayThumbnail;
		int syncVideoQuality; // 5 best to 1 worst, default 3 medium
		bool videoColor;

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

};
