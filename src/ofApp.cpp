#include "ofApp.h"

#include "../../common/src/Pinopticon.hpp"
#include "../../common/src/Pinopticon_Osc.hpp"
#include "../../common/src/Pinopticon_Http.hpp"

using namespace cv;
using namespace ofxCv;
using namespace Pinopticon;

//--------------------------------------------------------------
void ofApp::setup() {
    settings.loadFile("settings.xml");
    
    ofSetVerticalSync(false);
    ofHideCursor();

    appFramerate = settings.getValue("settings:app_framerate", 60);
    camFramerate = settings.getValue("settings:cam_framerate", 30);
    ofSetFrameRate(appFramerate);

    syncVideoQuality = settings.getValue("settings:osc_video_quality", 3); 
    videoColor = (bool) settings.getValue("settings:video_color", 0); 
    
    width = settings.getValue("settings:width", 640);
    height = settings.getValue("settings:height", 480);
    ofSetWindowShape(width, height);

    thumbWidth = settings.getValue("settings:thumb_width", 120);
    thumbHeight = settings.getValue("settings:thumb_height", 90);

    debug = (bool) settings.getValue("settings:debug", 1);

    sendOsc = (bool) settings.getValue("settings:send_osc", 1); 
    sendWs = (bool) settings.getValue("settings:send_ws", 1); 
    sendHttp = (bool) settings.getValue("settings:send_http", 1); 
    sendMjpeg = (bool) settings.getValue("settings:send_mjpeg", 1); 
    
    syncVideo = (bool) settings.getValue("settings:sync_video", 0); 
    blobs = (bool) settings.getValue("settings:blobs", 1);
    contours = (bool) settings.getValue("settings:contours", 0); 
    contourSlices = settings.getValue("settings:contour_slices", 10); 
    brightestPixel = (bool) settings.getValue("settings:brightest_pixel", 0); 

    oscHost = settings.getValue("settings:osc_host", "127.0.0.1");
    oscPort = settings.getValue("settings:osc_port", 7110);
    streamPort = settings.getValue("settings:stream_port", 7111);
    wsPort = settings.getValue("settings:ws_port", 7112);
    postPort = settings.getValue("settings:post_port", 7113);

    usePiCam = (bool) settings.getValue("settings:use_pi_cam", 1);
    rpiCamVersion = settings.getValue("settings:rpi_cam_version", 2);
    stillCompression = settings.getValue("settings:still_compression", 100);
    
    useUsbCam = (bool) settings.getValue("settings:use_usb_cam", 0);
    camUsbId = settings.getValue("settings:cam_usb_id", 0);
    
    useMjpegIn = (bool) settings.getValue("settings:use_mjpeg_in", 0);
    mjpegOutDirect = (bool) settings.getValue("settings:mjpeg_out_direct", 1);
    mjpegUrl = settings.getValue("settings:mjpeg_url", "http://nfg-rpi-3-4.local:7111/ipvideo");

    // OF_BLENDMODE_DISABLED, OF_BLENDMODE_ALPHA, OF_BLENDMODE_ADD, 
    // OF_BLENDMODE_SUBTRACT, OF_BLENDMODE_MULTIPLY, OF_BLENDMODE_SCREEN
    comboBlend = settings.getValue("settings:combo_blend", "screen");
    if (comboBlend == "add") {
        comboBlendMode = OF_BLENDMODE_ADD;
    } else if (comboBlend == "subtract") {
        comboBlendMode = OF_BLENDMODE_SUBTRACT;
    } else if (comboBlend == "multiply") {
        comboBlendMode = OF_BLENDMODE_MULTIPLY;
    } else {
        comboBlendMode = OF_BLENDMODE_SCREEN;
    }
    
    // camera
    if (videoColor) {
        gray.allocate(width, height, OF_IMAGE_COLOR);
        grayThumbnail.allocate(width, height, OF_IMAGE_COLOR);
		piCamTarget.allocate(width, height, OF_IMAGE_COLOR);
        camUsbTarget.allocate(width, height, OF_IMAGE_COLOR);
        mjpegInTarget.allocate(width, height, OF_IMAGE_COLOR);
    } else {
        gray.allocate(width, height, OF_IMAGE_GRAYSCALE);        
        grayThumbnail.allocate(width, height, OF_IMAGE_GRAYSCALE);     
		piCamTarget.allocate(width, height, OF_IMAGE_GRAYSCALE);
        camUsbTarget.allocate(width, height, OF_IMAGE_GRAYSCALE);
        mjpegInTarget.allocate(width, height, OF_IMAGE_GRAYSCALE);           
    }
    
    camMode = CamMode::None;

    if (usePiCam) {
        camMode = CamMode::PiOnly;

        cam.setup(width, height, camFramerate, videoColor); // color/gray;

        camRotation = settings.getValue("settings:cam_rotation", 0); 
        camSharpness = settings.getValue("settings:sharpness", 0); 
        camContrast = settings.getValue("settings:contrast", 0); 
        camBrightness = settings.getValue("settings:brightness", 50); 
        camIso = settings.getValue("settings:iso", 300); 
        camExposureMode = settings.getValue("settings:exposure_mode", 0); 
        camExposureCompensation = settings.getValue("settings:exposure_compensation", 0); 
        camShutterSpeed = settings.getValue("settings:shutter_speed", 0);

        cam.setRotation(camRotation);
        cam.setSharpness(camSharpness);
        cam.setContrast(camContrast);
        cam.setBrightness(camBrightness);
        cam.setISO(camIso);
        cam.setExposureMode((MMAL_PARAM_EXPOSUREMODE_T) camExposureMode);
        cam.setExposureCompensation(camExposureCompensation);
        cam.setShutterSpeed(camShutterSpeed);
        //cam.setFrameRate // not implemented in ofxCvPiCam 
    }
    
    if (useUsbCam) {
        if (camMode == CamMode::PiOnly) {
            camMode = CamMode::Combo;
        } else {
            camMode = CamMode::UsbOnly;
        }
        
        grabberSetup(camUsbId, camFramerate, width, height);   
    }
    
    if (useMjpegIn) {
        if (camMode == CamMode::PiOnly || camMode == CamMode::UsbOnly) {
            camMode = CamMode::Combo;
        } else {
            camMode = CamMode::MjpegOnly;
        }
        
        camIp.setURI(mjpegUrl);
        camIp.connect();
    }
    
    // ~ ~ ~   get a persistent name for this computer   ~ ~ ~
    // a randomly generated id
    sessionId = getSessionId();
   
    // the actual RPi hostname
    ofSystem("cp /etc/hostname " + ofToDataPath("DocumentRoot/js/"));
    hostName = getHostName();
    
    targetBlendFbo.allocate(width, height, GL_RGBA);
    targetBlendPixels.allocate(width, height, OF_IMAGE_COLOR);

    screenFbo.allocate(width, height, GL_RGBA);
    screenPixels.allocate(width, height, OF_IMAGE_COLOR);
    //fbo.allocate(width, height, GL_RGBA);
    pixels.allocate(width, height, OF_IMAGE_COLOR);
        
    thresholdValue = settings.getValue("settings:threshold", 127); 
    contourThreshold = 2.0;
    contourMinAreaRadius = 1.0;
    contourMaxAreaRadius = 250.0;   
    simplify = settings.getValue("settings:simplify", 0.5);
    smooth = settings.getValue("settings:smooth", 2);
    contourFinder.setMinAreaRadius(contourMinAreaRadius);
    contourFinder.setMaxAreaRadius(contourMaxAreaRadius);
    //contourFinder.setInvert(true); // find black instead of white
    trackingColorMode = TRACK_COLOR_RGB;

    // * stream video *
    int maxClientConnections = settings.getValue("settings:max_stream_connections", 5); // default 5
    int maxClientBitRate = settings.getValue("settings:max_stream_bitrate", 512); // default 1024
    int maxClientFrameRate = settings.getValue("settings:max_stream_framerate", 30); // default 30
    int maxClientQueueSize = settings.getValue("settings:max_stream_queue", 10); // default 10
    setupMjpeg(streamServer, streamPort, maxClientConnections, maxClientBitRate, maxClientFrameRate, maxClientQueueSize, width, height, "live_view.html");

    // * post form *
    setupHttp(this, postServer, postPort, "result.html");

    // * websockets *
    // events: connect, open, close, idle, message, broadcast
    setupWsServer(this, wsServer, wsPort);

    setupOscSender(sender, oscHost, oscPort);

    cout << "~ ~ ~ ~ ~ ~ ~ ~ ~ ~" << endl;
    cout << "Using Pi cam: " << usePiCam << " | version: " << rpiCamVersion << endl;
    cout << "Using USB cam: " << useUsbCam << " | id: " << camUsbId << endl;
    cout << "Using MJPEG in: " << useMjpegIn << " | MJPEG send mode: " << mjpegOutDirect << endl;
    cout << "Combo blend mode: " << comboBlend << endl;
    cout << "~ ~ ~ ~ ~ ~ ~ ~ ~ ~" << endl;
}

void ofApp::grabberSetup(int _id, int _fps, int _width, int _height) {
    //get back a list of devices.
    vector<ofVideoDevice> devices = camUsb.listDevices();

    for(size_t i = 0; i < devices.size(); i++){
        if(devices[i].bAvailable){
            //log the device
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName;
        }else{
            //log the device and note it as unavailable
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName << " - unavailable ";
        }
    }

    camUsb.setDeviceID(_id);
    camUsb.setDesiredFrameRate(_fps);
    camUsb.initGrabber(_width, _height);
}

//--------------------------------------------------------------
void ofApp::update() {
    timestamp = getTimestamp();
    newFrameToProcess = false;
    ofBackground(0);
       
    if (usePiCam) {
        frame_first = cam.grab();
        if (!frame_first.empty()) {
            if (camMode == CamMode::PiOnly) {
                toOf(frame_first, gray.getPixelsRef());
            } else {
                toOf(frame_first, piCamTarget.getPixelsRef());                
            }
            newFrameToProcess = true;
        }
    } 
    
    if (useUsbCam) {
        camUsb.update();
        if (camUsb.isFrameNew()) {  
            if (camMode == CamMode::UsbOnly) {
                gray.setFromPixels(camUsb.getPixelsRef());
            } else {
                camUsbTarget.setFromPixels(camUsb.getPixelsRef());                
            }                     
            newFrameToProcess = true;
        }
    } 
    
    if (useMjpegIn) {
        camIp.update();
        if (camIp.isFrameNew()) {
            if (camMode == CamMode::MjpegOnly) {
                gray.setFromPixels(camIp.getPixels());;
            } else {
                mjpegInTarget.setFromPixels(camIp.getPixels());;               
            }  
            newFrameToProcess = true;
        }
    }
    
    if (newFrameToProcess) {
        if (camMode == CamMode::Combo) {
            targetBlendFbo.begin();
             ofEnableBlendMode(OF_BLENDMODE_ALPHA); 

            if (comboBlendMode == OF_BLENDMODE_SUBTRACT || comboBlendMode == OF_BLENDMODE_MULTIPLY) {
                ofBackground(255);
            } else {
                ofBackground(0);
            }
            
            ofEnableBlendMode(comboBlendMode); 
            if (usePiCam) piCamTarget.draw(0,0);  
            if (useUsbCam) camUsbTarget.draw(0,0);
            if (useMjpegIn) mjpegInTarget.draw(0,0);
            targetBlendFbo.end();

            targetBlendFbo.readToPixels(targetBlendPixels);
            gray.setFromPixels(targetBlendPixels);
        }
        
        frame = toCv(gray);

        if (mjpegOutDirect) {
            //std::cout << ("Sending direct") << endl;
            if (sendMjpeg) streamServer.send(gray.getPixels());
            
            if (syncVideo) {
                grayThumbnail.setFromPixels(gray.getPixels());
                grayThumbnail.resize(thumbWidth, thumbHeight);
                imageToBuffer(grayThumbnail, videoBuffer, syncVideoQuality);

                if (sendOsc) sendOscVideo(sender, hostName, sessionId, videoBuffer, timestamp);
                if (sendWs) sendWsVideo(wsServer, hostName, sessionId, videoBuffer, timestamp);
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw() {
    if(newFrameToProcess) {
        screenFbo.begin();
        ofBackground(0);
        
        if (debug) {
            if (!blobs && !contours) {
                drawMat(frame, 0, 0);
            } else if (blobs || contours) {
                drawMat(frameProcessed, 0, 0);
            }
        }

        if (blobs) {
            if (debug) {
            	ofSetLineWidth(2);
            	ofNoFill();
            }
            
            //autothreshold(frameProcessed);        
            threshold(frame, frameProcessed, thresholdValue, 255, 0);
            contourFinder.setThreshold(contourThreshold);    
            contourFinder.findContours(frameProcessed);

            int n = contourFinder.size();
            for (int i = 0; i < n; i++) {
                float circleRadius;
                glm::vec2 circleCenter = toOf(contourFinder.getMinEnclosingCircle(i, circleRadius));
                if (debug) {
                	ofSetColor(cyanPrint);
                	ofDrawCircle(circleCenter, circleRadius);
                	ofDrawCircle(circleCenter, 1);
                }

                if (sendOsc) sendOscBlobs(sender, hostName, sessionId, i, circleCenter.x, circleCenter.y, timestamp);
                if (sendWs) sendWsBlobs(wsServer, hostName, sessionId, i, circleCenter.x, circleCenter.y, timestamp);
            }
        }

        if (contours) {
            if (debug) {
                ofSetLineWidth(2);
                ofNoFill();
            }

            int contourCounter = 0;
            unsigned char * pixels = gray.getPixels().getData();
            int gw = gray.getWidth();

            for (int h=0; h<255; h += int(255/contourSlices)) {
                contourFinder.setThreshold(h);
                contourFinder.findContours(frame);
                if (debug) contourFinder.draw();            

                int n = contourFinder.size();
                for (int i = 0; i < n; i++) {
                    ofPolyline line = contourFinder.getPolyline(i);
                    line.simplify(simplify);
                    line = line.getSmoothed(smooth, 0.5);
                    vector<glm::vec3> cvPoints = line.getVertices();

                    int x = int(cvPoints[0].x);
                    int y = int(cvPoints[0].y);
                    ofColor col = pixels[x + y * gw];
                    float colorData[3]; 
                    colorData[0] = col.r;
                    colorData[1] = col.g;
                    colorData[2] = col.b;
                    char const * pColor = reinterpret_cast<char const *>(colorData);
                    std::string colorString(pColor, pColor + sizeof colorData);
                    contourColorBuffer.set(colorString); 

                    float z = col.getBrightness();
                    float pointsData[cvPoints.size() * 3]; 
                    for (int j=0; j<cvPoints.size(); j++) {
                        int index = j * 3;
                        pointsData[index] = cvPoints[j].x;
                        pointsData[index+1] = cvPoints[j].y;
                        pointsData[index+2] = z; ///cvPoints[j].z;
                    }
                    char const * pPoints = reinterpret_cast<char const *>(pointsData);
                    std::string pointsString(pPoints, pPoints + sizeof pointsData);
                    contourPointsBuffer.set(pointsString); 

                    if (sendOsc) sendOscContours(sender, hostName, sessionId, contourCounter, contourColorBuffer, contourPointsBuffer, timestamp);
                    if (sendWs) sendWsContours(wsServer, hostName, sessionId, contourCounter, contourColorBuffer, contourPointsBuffer, timestamp);
                    contourCounter++;
                }        
            }
        }
           
        if (brightestPixel) {
        	// this mostly useful as a performance baseline
            // https://openframeworks.cc/ofBook/chapters/image_processing_computer_vision.html
            float maxBrightness = 0; 
            float maxBrightnessX = 0; 
            float maxBrightnessY = 0;
            int skip = 2;

            for (int y=0; y<height - skip; y += skip) {
                for (int x=0; x<width - skip; x += skip) {
                    ofColor colorAtXY = gray.getColor(x, y);
                    float brightnessOfColorAtXY = colorAtXY.getBrightness();
                    if (brightnessOfColorAtXY > maxBrightness && brightnessOfColorAtXY > thresholdValue) {
                        maxBrightness = brightnessOfColorAtXY;
                        maxBrightnessX = x;
                        maxBrightnessY = y;
                    }
                }
            }

            if (debug) {
            	ofNoFill();
            	glm::vec2 circleCenter = glm::vec2(maxBrightnessX, maxBrightnessY);
            	ofDrawCircle(circleCenter, 40);
            }

            if (sendOsc) sendOscPixel(sender, hostName, sessionId, maxBrightnessX, maxBrightnessY, timestamp);
            if (sendWs) sendWsPixel(wsServer, hostName, sessionId, maxBrightnessX, maxBrightnessY, timestamp);
        }
        
        screenFbo.end();

        if (!mjpegOutDirect && sendMjpeg || syncVideo) {           
            //std::cout << ("Sending fbo") << endl;

            screenFbo.readToPixels(screenPixels);
            if (sendMjpeg) streamServer.send(screenPixels);
            
            if (syncVideo) {
                grayThumbnail.setFromPixels(screenPixels);
                grayThumbnail.resize(thumbWidth, thumbHeight);
                imageToBuffer(grayThumbnail, videoBuffer, syncVideoQuality);

                if (sendOsc) sendOscVideo(sender, hostName, sessionId, videoBuffer, timestamp);
                if (sendWs) sendWsVideo(wsServer, hostName, sessionId, videoBuffer, timestamp);            
            }
        }
    }
       
    if (debug) {
        screenFbo.draw(0,0);

        stringstream info;
        info << cam.width << "x" << cam.height << " @ "<< ofGetFrameRate() <<"fps"<< "\n";
        ofDrawBitmapStringHighlight(info.str(), 10, 10, ofColor::black, ofColor::yellow);
    }
}

// ~ ~ ~ POST ~ ~ ~
void ofApp::onHTTPPostEvent(ofxHTTP::PostEventArgs& args) {
    ofLogNotice("ofApp::onHTTPPostEvent") << "Data: " << args.getBuffer().getText();

    takePhoto();
}


void ofApp::onHTTPFormEvent(ofxHTTP::PostFormEventArgs& args) {
    ofLogNotice("ofApp::onHTTPFormEvent") << "";
    ofxHTTP::HTTPUtils::dumpNameValueCollection(args.getForm(), ofGetLogLevel());
    
    takePhoto();
}


void ofApp::onHTTPUploadEvent(ofxHTTP::PostUploadEventArgs& args) {
    std::string stateString = "";

    switch (args.getState()) {
        case ofxHTTP::PostUploadEventArgs::UPLOAD_STARTING:
            stateString = "STARTING";
            break;
        case ofxHTTP::PostUploadEventArgs::UPLOAD_PROGRESS:
            stateString = "PROGRESS";
            break;
        case ofxHTTP::PostUploadEventArgs::UPLOAD_FINISHED:
            stateString = "FINISHED";
            break;
    }

    ofLogNotice("ofApp::onHTTPUploadEvent") << "";
    ofLogNotice("ofApp::onHTTPUploadEvent") << "         state: " << stateString;
    ofLogNotice("ofApp::onHTTPUploadEvent") << " formFieldName: " << args.getFormFieldName();
    ofLogNotice("ofApp::onHTTPUploadEvent") << "orig. filename: " << args.getOriginalFilename();
    ofLogNotice("ofApp::onHTTPUploadEvent") <<  "     filename: " << args.getFilename();
    ofLogNotice("ofApp::onHTTPUploadEvent") <<  "     fileType: " << args.getFileType().toString();
    ofLogNotice("ofApp::onHTTPUploadEvent") << "# bytes xfer'd: " << args.getNumBytesTransferred();
}

// ~ ~ ~ WEBSOCKETS ~ ~ ~
void ofApp::onWebSocketOpenEvent(ofxHTTP::WebSocketEventArgs& evt) {
    cout << "Websocket connection opened." << evt.connection().clientAddress().toString() << endl;
}

void ofApp::onWebSocketCloseEvent(ofxHTTP::WebSocketCloseEventArgs& evt) {
    cout << "Websocket connection closed." << evt.connection().clientAddress().toString() << endl;
}

void ofApp::onWebSocketFrameReceivedEvent(ofxHTTP::WebSocketFrameEventArgs& evt) {
    cout << "Websocket frame was received:" << evt.connection().clientAddress().toString() << endl;
    string msg = evt.frame().getText();
    cout <<  msg << endl;

    if (msg == "take_photo") {
        takePhoto();
    } else if (msg == "stream_photo") {
        streamPhoto();
    }
}

void ofApp::onWebSocketFrameSentEvent(ofxHTTP::WebSocketFrameEventArgs& evt) {
    cout << "Websocket frame was sent." << endl;
}


void ofApp::onWebSocketErrorEvent(ofxHTTP::WebSocketErrorEventArgs& evt) {
    cout << "Websocket Error." << evt.connection().clientAddress().toString() << endl;
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
void ofApp::createResultHtml(string fileName) {
    string photoIndexFileName = "DocumentRoot/result.html";
    ofBuffer buff;
    ofFile photoIndexFile;
    photoIndexFile.open(ofToDataPath(photoIndexFileName), ofFile::ReadWrite, false);

    string photoIndex = "<!DOCTYPE html>\n";
    
    if (fileName == "none") { // use existing file if it's there
        photoIndex += "<html><head><meta http-equiv=\"refresh\" content=\"0\"></head><body>\n";
        photoIndex += "WAIT\n";
    } else { // otherwise make a new one
        photoIndex += "<html><head></head><body>\n";
        
        string lastFile = ofFilePath::getFileName(fileName);

        //lastPhotoTakenName = ofFilePath::getFileName(fileName);
        lastPhotoTakenName = hostName + "_" + lastFile;
        ofSystem("mv " + ofToDataPath("DocumentRoot/photos/" + lastFile) + " " + ofToDataPath("DocumentRoot/photos/" + lastPhotoTakenName));
        
        photoIndex += "<a href=\"photos/" + lastPhotoTakenName + "\">" + lastPhotoTakenName + "</a>\n";
    }

    photoIndex += "</body></html>\n";

    buff.set(photoIndex.c_str(), photoIndex.size());
    ofBufferToFile(photoIndexFileName, buff);
}

void ofApp::takePhoto() {
    ofSaveImage(gray, photoBuffer, OF_IMAGE_FORMAT_JPEG, OF_IMAGE_QUALITY_BEST);
    string fileName = "photo_" + ofToString(timestamp) + ".jpg";
    ofBufferToFile(ofToDataPath("DocumentRoot/photos/") + fileName, photoBuffer);
    createResultHtml(fileName);

    //string msg = "{\"unique_id\":" + sessionId + ",\"hostname\":" + hostName + ",\"photo\":" + ofxCrypto::base64_encode(photoBuffer) + ",\"timestamp\":" + ofToString(timestamp) + "}";
    string msg = hostName + "," + lastPhotoTakenName;
    wsServer.webSocketRoute().broadcast(ofxHTTP::WebSocketFrame(msg));
}

void ofApp::streamPhoto() {
    ofSaveImage(gray, photoBuffer, OF_IMAGE_FORMAT_JPEG, OF_IMAGE_QUALITY_BEST);
    //string fileName = "photo_" + ofToString(timestamp) + ".jpg";
    //ofBufferToFile(ofToDataPath("DocumentRoot/photos/") + fileName, photoBuffer);
    //createResultHtml(fileName);

    string photo64 = ofxCrypto::base64_encode(photoBuffer);
    string msg = "{\"unique_id\":\"" + sessionId + "\",\"hostname\":\"" + hostName + "\",\"photo\":\"" + photo64 + "\",\"timestamp\":\"" + ofToString(timestamp) + "\"}";
    wsServer.webSocketRoute().broadcast(ofxHTTP::WebSocketFrame(msg));
}

