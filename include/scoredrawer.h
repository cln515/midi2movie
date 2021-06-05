#pragma once
#define _USE_MATH_DEFINES
#include <iostream>
#include <MidiFile.h>
#include <Windows.h>
#include <gl\GL.h>
#include <gl\GLU.h>
#include "Eigen/Eigen"
#include "Eigen/Core"
#include <mapping.h>

class scoreVis {
public:
	scoreVis() {};
	void render(smf::MidiFile midifile, GLubyte*& colorImage, double time);
	void createContext(int view_w, int view_h);
	void InitPers(int viewWidth, int viewHeight, double znear, double depthResolution, double* intrinsic);

	void setMapping(mapping* m_) { m = m_; }
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

	mapping* m;
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