#pragma once
#define _USE_MATH_DEFINES
#include <iostream>


class mapping{
public:
	mapping() {};
    virtual void getpos(int note,int track,double* posinfo);
	virtual void getpos(double freq, double* posinfo);

private:
};

class straightMapping : public mapping {
public:
	straightMapping(double zitv_) {
		zitv = zitv_;
	};
	void getpos(int note, int track, double* posinfo);
	void getpos(double freq, double* posinfo);
	double zitv = 0.5;
};


class circleMapping : public mapping {
public:
	circleMapping(double zitv_,double diag_,double startAngle_,double resolution_) {
		zitv = zitv_;
		diag = diag_;
		startAngle = startAngle_;
		resolution = resolution_;
	};
	void getpos(int note, int track, double* posinfo);
	void getpos(double freq, double* posinfo);
	double zitv = 0.05;
	double diag = 1.0;
	double startAngle = 0;
	double resolution=M_PI/127;
};