#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){	 
	
	ofSetVerticalSync(true);
	ofSetCircleResolution(80);
	ofBackground(54, 54, 54);	
	
	// 0 output channels, 
	// 2 input channels
	// 44100 samples per second
	// 256 samples per buffer
	// 4 num buffers (latency)
	
	soundStream.listDevices();
	soundStream.setDeviceID(0);//this now uses the audio input rather than mic input for mac 

	//outputStream.setDeviceID(1);
	
	//if you want to set a different device id 
	//soundStream.setDeviceID(0); //bear in mind the device id corresponds to all audio devices, including  input-only and output-only devices.
	
	bufferSize = 256;
	
	
	left.assign(bufferSize, 0.0);
	right.assign(bufferSize, 0.0);
	volHistory.assign(400, 0.0);
	
	bufferCounter	= 0;
	drawCounter		= 0;
	smoothedVol     = 0.0;
	scaledVol		= 0.0;

	soundStream.setup(this, 0, 2, 44100, bufferSize, 4);
	
	fftSize = 512;
	fftHopsize = fftSize/2;
	
	audiofft = new accFFT(fftSize, 1);//type 1 is fft with double precision
	
	fftTest();
	fftInput = new double[fftSize];
	fftOutput = new fft_complex_double[fftSize];//float[bufferSize][2];

	fftWindow = new double[fftSize];
	set_window_hanning(fftWindow, fftSize);
	
}

void testApp::fftTest(){
 accFFT fft(8,1);
 
 double my_buff[8] = {1,4,2,1,1,7,7,9};
 //    double real[8];
 //  double complex[8];
 
 double complexResult[8][2];
 
 fft.forward_FFT_d(my_buff, complexResult);
	

 
 
 for (int i = 0;i < 8;i++)
 {
 cout << "real " << complexResult[i][0] << " imag  " << complexResult[i][1] << endl;
 }
 
}


void testApp :: set_window_hanning(double* windowBuffer, const int framesize){
	double N;		// variable to store framesize minus 1
	
	N = (double) (framesize-1);	// framesize minus 1
	
	// Hanning window calculation
	for (int n = 0;n < framesize;n++)
	{
		windowBuffer[n] = 0.5*(1-cos(2*pi*(n/N)));
	}
}

//--------------------------------------------------------------
void testApp::update(){
	//lets scale the vol up to a 0-1 range 
	scaledVol = ofMap(smoothedVol, 0.0, 0.17, 0.0, 1.0, true);

	//lets record the volume into an array
	volHistory.push_back( scaledVol );
	
	//if we are bigger the the size we want to record - lets drop the oldest value
	if( volHistory.size() >= 400 ){
		volHistory.erase(volHistory.begin(), volHistory.begin()+1);
	}
}

//--------------------------------------------------------------
void testApp::draw(){
	
	/*
	ofSetColor(225);
	ofDrawBitmapString("AUDIO INPUT EXAMPLE", 32, 32);
	ofDrawBitmapString("press 's' to unpause the audio\n'e' to pause the audio", 31, 92);
	
	ofNoFill();
	
	// draw the left channel:
	ofPushStyle();
		ofPushMatrix();
		ofTranslate(32, 170, 0);
			
		ofSetColor(225);
		ofDrawBitmapString("Left Channel", 4, 18);
		
		ofSetLineWidth(1);	
		ofRect(0, 0, 512, 200);

		ofSetColor(245, 58, 135);
		ofSetLineWidth(3);
					
			ofBeginShape();
			for (int i = 0; i < left.size(); i++){
				ofVertex(i*2, 100 -left[i]*180.0f);
			}
			ofEndShape(false);
			
		ofPopMatrix();
	ofPopStyle();

	// draw the right channel:
	ofPushStyle();
		ofPushMatrix();
		ofTranslate(32, 370, 0);
			
		ofSetColor(225);
		ofDrawBitmapString("Right Channel", 4, 18);
		
		ofSetLineWidth(1);	
		ofRect(0, 0, 512, 200);

		ofSetColor(245, 58, 135);
		ofSetLineWidth(3);
					
			ofBeginShape();
			for (int i = 0; i < right.size(); i++){
				ofVertex(i*2, 100 -right[i]*180.0f);
			}
			ofEndShape(false);
			
		ofPopMatrix();
	ofPopStyle();
	
	// draw the average volume:
	ofPushStyle();
		ofPushMatrix();
		ofTranslate(565, 170, 0);
			
		ofSetColor(225);
		ofDrawBitmapString("Scaled average vol (0-100): " + ofToString(scaledVol * 100.0, 0), 4, 18);
		ofRect(0, 0, 400, 400);
		
		ofSetColor(245, 58, 135);
		ofFill();		
		ofCircle(200, 200, scaledVol * 190.0f);
		
		//lets draw the volume history as a graph
		ofBeginShape();
		for (int i = 0; i < volHistory.size(); i++){
			if( i == 0 ) ofVertex(i, 400);

			ofVertex(i, 400 - volHistory[i] * 70);
			
			if( i == volHistory.size() -1 ) ofVertex(i, 400);
		}
		ofEndShape(false);		
			
		ofPopMatrix();
	ofPopStyle();
	
	drawCounter++;
	
	ofSetColor(225);
	string reportString = "buffers received: "+ofToString(bufferCounter)+"\ndraw routines called: "+ofToString(drawCounter)+"\nticks: " + ofToString(soundStream.getTickCount());
	ofDrawBitmapString(reportString, 32, 589);
	
	*/
	drawSpectrogram();
}


void testApp::drawSpectrogram(){
	int spectrogramWidth = 400;
	
	int minIndex = max(0, (int)(magnitudeSpectrum.size() - spectrogramWidth));
	int maxIndex = minIndex+spectrogramWidth;
	int screenHeight = ofGetHeight();
	
	float spectrogramHeight = 20.0;//bufferSize;
	float binWidth = (float)ofGetWidth()/(float)spectrogramWidth;
	float binHeight = (float)ofGetHeight()/spectrogramHeight;
	
	for (int i = minIndex; i < min((int)magnitudeSpectrum.size(), maxIndex ); i++){
		for (int y = 0; y < spectrogramHeight; y++){
			ofSetColor(0, magnitudeSpectrum[i][y]*40.0, 0);
			ofRect((i-minIndex)*binWidth, screenHeight - (y+1)*binHeight, binWidth, binHeight);
		}
	}
}


//--------------------------------------------------------------
void testApp::audioIn(float * input, int bufferSize, int nChannels){	
	
	float curVol = 0.0;
	
	// samples are "interleaved"
	int numCounted = 0;	

	//lets go through each sample and calculate the root mean square which is a rough way to calculate volume	
	for (int i = 0; i < bufferSize; i++){
		left[i]		= input[i*2]*0.5;
		
		addToFFT(left[i]);
		
		
		
		right[i]	= input[i*2+1]*0.5;

		curVol += left[i] * left[i];
		curVol += right[i] * right[i];
		numCounted+=2;
	}
	
	
	//this is how we get the mean of rms :) 
	curVol /= (float)numCounted;
	
	// this is how we get the root of rms :) 
	curVol = sqrt( curVol );
	
	smoothedVol *= 0.93;
	smoothedVol += 0.07 * curVol;
	
	bufferCounter++;
	
	
}

void testApp::addToFFT(float sample){
	//window
	fftSampleBuffer.push_back(sample);
	
	if (fftSampleBuffer.size() == fftSize){
		setFFTinput();
		//do FFT using Accelerate
		audiofft->forward_FFT_d(fftInput, fftOutput);//Result);//&fftRealResult[0], &fftImagResult[0]);
		
		DoubleVector v;
		for (int i = 0; i < bufferSize; i++){
			v.push_back(sqrt(fftOutput[i][0]*fftOutput[i][0] + fftOutput[i][1]*fftOutput[i][1]));
		}
		magnitudeSpectrum.push_back(v);
		/*
		 //see what it's doing for low frequencies
		 for (int j = 0; j < 4; j++){
		 printf("%i %.3f, ", j, v[j]);
		 }
		 printf("\n");
		 */
		
		//now remove many samples
		for (int removeIndex = 0; removeIndex < fftHopsize; removeIndex++){
			fftSampleBuffer.erase(fftSampleBuffer.begin());//just need first to go each time
		}
	}
}

void testApp::setFFTinput(){
	for (int i = 0; i < fftSize; i++){
		fftInput[i] = fftSampleBuffer[i] * fftWindow[i];
	}
}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){ 
	if( key == 's' ){
		soundStream.start();
	}
	
	if( key == 'e' ){
		soundStream.stop();
	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){ 
	
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}

