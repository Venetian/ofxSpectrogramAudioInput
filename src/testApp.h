#ifndef _TEST_APP
#define _TEST_APP


#include "ofMain.h"

#include "accFFT.h"

//note - also requires the Accelerate framework (add existing frameworks)
typedef double fft_complex_double[2];

class testApp : public ofBaseApp{
	
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
				
		void audioIn(float * input, int bufferSize, int nChannels); 
	
		vector <float> left;
		vector <float> right;
		vector <float> volHistory;
		
		int 	bufferCounter;
		int 	drawCounter;
		
		float smoothedVol;
		float scaledVol;
		
		ofSoundStream soundStream;

	void fftTest();
	
	//float* fftInput;//[256];
	int bufferSize;
	int fftSize, fftHopsize;
	fft_complex_double* fftOutput;
	accFFT* audiofft;//((int)256, (int)1);
	
	
	double* fftInput;
	double* fftWindow;
	
	typedef std::vector<double> DoubleVector;
	typedef std::vector<DoubleVector> DoubleMatrix;
	DoubleVector fftSampleBuffer;
	DoubleMatrix magnitudeSpectrum;
	void addToFFT(float sample);
	void setFFTinput();
	void set_window_hanning(double* windowBuffer, const int framesize);

	void drawSpectrogram();
	
};

#endif	

