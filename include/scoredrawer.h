#pragma once
#define _USE_MATH_DEFINES
#include <iostream>
#include <MidiFile.h>

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#else
// Linux and all others
// Using GCC 4 where hiding attributes is possible
//#include <GL/glew.h>
#endif


//#include <gl\GL.h>
//#include <gl\GLU.h>
#include <GLFW/glfw3.h>
#include "Eigen/Eigen"
#include "Eigen/Core"
#include <mapping.h>

class scoreVis {
public:
	scoreVis() {};

	void setView();
	void render_spectrum();
	void render(smf::MidiFile midifile, double time);

	void bufferCopy(GLubyte*& colorImage);

	void createContext(int view_w, int view_h);
	void InitPers(int viewWidth, int viewHeight, double znear, double depthResolution, double* intrinsic);

	void setMapping(mapping* m_) { m = m_; }
	void setSpecMapping(mapping* m_) { m_f = m_; }
	void setRotation(Eigen::Matrix3d R_) { R = R_; }
	void setXYZ(double x, double y, double z) {
		tx_ = x;
		ty_ = y;
		tz_ = z;
	};
	void setSpeed(double speed) {
		spd = speed;
	}

	
	void setSpectrum(double* buffer, int n,int minf,int maxf) {
		setSpec = true;
		spectrum = buffer;
		specLen = n;
		minf_idx = minf;
		maxf_idx = maxf;
	}
	void setLineColor(unsigned char* linecolors_) {
		linecolors = linecolors_;
	};

	unsigned char* linecolors;

	mapping* m;
	mapping* m_f;
	GLFWwindow* win;
	Eigen::Matrix3d R;
	double tx_;
	double ty_;
	double tz_;
	double spd;

	int viewWidth_ = 1280;
	int viewHeight_ = 720;
	double znear = 0.1;
	double depthResolution = 20.0;
	double intrinsic[4] = { 640, 360 ,200, 200 };

	double dur = 0;
	bool setSpec = false;
	double* spectrum;
	int specLen=0;
	int minf_idx,maxf_idx;
};

void HSVAngle2Color(double radangle, unsigned char* rgb);
void trackColor(int track, unsigned char* rgb, unsigned char* rgb2);