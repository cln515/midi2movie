#include <mapping.h>

void mapping::getpos(int note, int track, double* posinfo) {

	//straight line
	double dy = note * 0.05;
	double dz = track * 0.5;

	posinfo[0] = dy;
	posinfo[1] = dz;
	posinfo[2] = dy + 0.05;
	posinfo[3] = dz;

}

void mapping::getpos(double freq, double* posinfo) {

	//straight line
	double dy = freq;
	double dz = 0;

	posinfo[0] = dy;
	posinfo[1] = dz;
	posinfo[2] = dy + 0.05;
	posinfo[3] = dz;

}

void straightMapping::getpos(int note, int track, double* posinfo) {

	//straight line
	double dy = note * 0.05;
	double dz = track * zitv;

	posinfo[0] = dy;
	posinfo[1] = dz;
	posinfo[2] = dy + 0.05;
	posinfo[3] = dz;

};

//spectrum
void straightMapping::getpos(double freq, double* posinfo) {

	//straight line
	double dy = (freq-0.5) * 0.05 * 100;

	posinfo[0] = dy;
	posinfo[1] = -1.5;
	posinfo[2] = 1;//line normal in yz-plane
	posinfo[3] = 0;//line normal in yz-plane
	posinfo[4] = 1;//line normal in yz-plane
	posinfo[5] = 0;

};

void circleMapping::getpos(int note, int track, double* posinfo) {

	//circle line
	double t0 = (note)*resolution;
	double t1 = (note + 1)*resolution;
	double dy = (diag -(zitv*track)) * cos(t0 + startAngle+M_PI/4);
	double dz = (diag - (zitv*track)) * sin(t0 + startAngle + M_PI / 4);
	double dy1 = (diag - (zitv*track)) * cos(t1 + startAngle + M_PI / 4);
	double dz1 = (diag - (zitv*track)) * sin(t1 + startAngle + M_PI / 4);

	posinfo[0] = dy;
	posinfo[1] = dz;
	posinfo[2] = dy1;
	posinfo[3] = dz1;
};


void circleMapping::getpos(double freq, double* posinfo) {

	//circle line
	double t0 = freq;

	double ny = cos(t0 * (2 * M_PI) + startAngle + M_PI / 4);
	double nz = sin(t0 * (2 * M_PI) + startAngle + M_PI / 4);
	double dy = (diag) * cos(t0 * (2* M_PI) + startAngle + M_PI / 4);
	double dz = (diag) * sin(t0 * (2 * M_PI) + startAngle + M_PI / 4);
	
	posinfo[0] = dy;
	posinfo[1] = dz;
	posinfo[2] = 1;//line normal in yz-plane
	posinfo[3] = ny;
	posinfo[4] = nz;
	posinfo[5] = 0;
};