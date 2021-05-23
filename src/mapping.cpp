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


void straightMapping::getpos(int note, int track, double* posinfo) {

	//straight line
	double dy = note * 0.05;
	double dz = track * zitv;

	posinfo[0] = dy;
	posinfo[1] = dz;
	posinfo[2] = dy + 0.05;
	posinfo[3] = dz;

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