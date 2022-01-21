#include "scoredrawer.h"

void scoreVis::createContext(int view_w, int view_h) {
	viewWidth_ = view_w;
	viewHeight_ = view_h;

	if (::glfwInit() == GL_FALSE) {
		std::cout << "glfwInit() error" << std::endl;
		exit(0);
	}
	::glfwWindowHint(GLFW_VISIBLE, 0);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	win = ::glfwCreateWindow(view_w, view_h, "", NULL, NULL);
	glfwMakeContextCurrent(win);
	::glfwGetWindowSize(win, &viewWidth_, &viewHeight_);
	std::cout << viewWidth_ << "," << viewHeight_ << std::endl;
}

void scoreVis::setView() {
	InitPers(viewWidth_, viewHeight_, znear, depthResolution, intrinsic);
}

void scoreVis::render_spectrum() {
	if (setSpec) {
		double pos[6];
		glLineWidth(3.0);
		glBegin(GL_LINES);
		for (int i = minf_idx; i <= maxf_idx; i++) {

			Eigen::Vector3d v1, v2, v3, v4;
			double specpower = 10 * log10(sqrt(spectrum[i] * spectrum[i]) / specLen + 1);//base power setting
			m_f->getpos((double)i / (maxf_idx - minf_idx), pos);


			v1 << pos[0], pos[1], pos[2];
			v2 << pos[0] + specpower * pos[3], pos[1] + specpower * pos[4], pos[2] + specpower * pos[5];

			//v1 = R.transpose() * (v1-t_fix);
			//v2 = R.transpose() * (v2 - t_fix);

			glColor3ub(linecolors[0], linecolors[1], linecolors[2]);
			glVertex3f(v1(0), v1(1), v1(2));
			glColor3ub(linecolors[0], linecolors[1], linecolors[2]);
			glVertex3f(v2(0), v2(1), v2(2));

		}
		glEnd();
	}
}

void scoreVis::render(smf::MidiFile midifile, double time) {
	//GLint view[4];
	{
		//glGetIntegerv(GL_VIEWPORT, view);
		double pos[6];
		Eigen::Vector3d t_fix;
		t_fix << tx_, ty_, tz_;
		//spectrum

        //note drawing

		Eigen::Vector3d t;
		t << time*spd+ tx_ , ty_, tz_;

		glBegin(GL_TRIANGLES);
		unsigned char trackColors[3], trackColore[3];

		for (int track = 0; track < midifile.getTrackCount(); ++track) {
			trackColor(track, trackColors, trackColore);
			for (int event = 0; event < midifile[track].size(); ++event) {
				if (midifile[track][event].isNoteOn()) {
					double dx = midifile[track][event].seconds*spd;
					double length = midifile[track][event].getDurationInSeconds()*spd;

                    m->getpos(midifile[track][event][1],track,pos);
					Eigen::Vector3d v1, v2, v3, v4;
					v1 << dx,                   pos[0], pos[1];
					v2 << dx,                   pos[2], pos[3];
					v3 << dx + length,          pos[0], pos[1];
					v4 << dx + length,          pos[2], pos[3];

					v1 = R.transpose() * (v1 - t);
					v2 = R.transpose() * (v2 - t);
					v3 = R.transpose() * (v3 - t);
					v4 = R.transpose() * (v4 - t);

					unsigned char rs, gs, bs;
					unsigned char re, ge, be;
					if (midifile[track][event].seconds <time && midifile[track][event].seconds + length > time) {
						double rate = (time - midifile[track][event].seconds)/length;
						rs = trackColors[0]*rate + 255*(1-rate) ;						gs = trackColors[1] * rate + 255 * (1 - rate);						bs = trackColors[2] * rate + 255 * (1 - rate);
						re = trackColore[0] * rate + 255 * (1 - rate);						ge = trackColore[1] * rate + 255 * (1 - rate);						be = trackColore[2] * rate + 255 * (1 - rate);
					}else {
						rs = trackColors[0];						gs = trackColors[1];						bs = trackColors[2];
						re = trackColore[0];						ge = trackColore[1];						be = trackColore[2];
					}

					glColor3ub(rs, gs, bs);
					glVertex3f(v1(0), v1(1), v1(2));
					glColor3ub(rs, gs, bs);
					glVertex3f(v2(0), v2(1), v2(2));
					glColor3ub(re, ge, be);
					glVertex3f(v3(0), v3(1), v3(2));

					glColor3ub(rs, gs, bs);
					glVertex3f(v2(0), v2(1), v2(2));
					glColor3ub(re, ge, be);
					glVertex3f(v3(0), v3(1), v3(2));
					glColor3ub(re, ge, be);
					glVertex3f(v4(0), v4(1), v4(2));
				}
			}
		}
		glEnd();

		
	}
	


}

void scoreVis::bufferCopy(GLubyte*& colorImage) {
	GLint view[4];
	glGetIntegerv(GL_VIEWPORT, view);

	colorImage = (GLubyte*)malloc(sizeof(GLubyte)*view[2] * view[3] * 3);
	glReadPixels(view[0], view[1], view[2], view[3], GL_RGB, GL_UNSIGNED_BYTE, colorImage);

}


void scoreVis::InitPers(int viewWidth, int viewHeight, double znear, double depthResolution, double* intrinsic) {

	glViewport(0, 0, viewWidth, viewHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glEnable(GL_DEPTH_TEST);

	GLfloat m[16];

	Eigen::Matrix4d m1_, r2l, rev;//projection
	double cx = intrinsic[0];
	double cy = intrinsic[1];
	double fx = intrinsic[2];
	double fy = intrinsic[3];
	double zfar = znear + depthResolution;

	m1_ <<
		2 * fx / viewWidth, 0, -(viewWidth - 2 * cx) / viewWidth, 0,
		0, 2 * fy / viewHeight, -(viewHeight - 2 * cy) / viewHeight, 0,
		0, 0, (zfar + znear) / (zfar - znear), -2 * zfar*znear / (zfar - znear),
		0, 0, 1, 0;

	Eigen::Matrix4d m3 = m1_;

	GLdouble m2[16];
	memcpy(m2, m3.data(), sizeof(double) * 16);

	glMultMatrixd(m2);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void trackColor(int track, unsigned char* rgb, unsigned char* rgb2) {
	HSVAngle2Color(track*1.0, rgb);
	HSVAngle2Color(track*1.0 + 0.5, rgb2);
}

void HSVAngle2Color(double radangle, unsigned char* rgb) {
	double pi_sixtydig = M_PI / 3;
	double angle = ((radangle / (M_PI * 2)) - (int)(radangle / (M_PI * 2)))*(M_PI * 2);
	if (angle >= 0 && angle < pi_sixtydig) {
		double val = (angle - pi_sixtydig * 0) / pi_sixtydig;
		rgb[0] = 255;
		rgb[1] = 255 * val;
		rgb[2] = 0;
	}
	else if (angle >= pi_sixtydig * 1 && angle < pi_sixtydig * 2) {
		double val = (angle - pi_sixtydig * 1) / pi_sixtydig;
		rgb[0] = 255 * (1 - val);
		rgb[1] = 255;
		rgb[2] = 0;
	}
	else if (angle >= pi_sixtydig * 2 && angle < pi_sixtydig * 3) {
		double val = (angle - pi_sixtydig * 2) / pi_sixtydig;
		rgb[0] = 0;
		rgb[1] = 255;
		rgb[2] = 255 * (val);
	}
	else if (angle >= pi_sixtydig * 3 && angle < pi_sixtydig * 4) {
		double val = (angle - pi_sixtydig * 3) / pi_sixtydig;
		rgb[0] = 0;
		rgb[1] = 255 * (1 - val);
		rgb[2] = 255;
	}
	else if (angle >= pi_sixtydig * 4 && angle < pi_sixtydig * 5) {
		double val = (angle - pi_sixtydig * 4) / pi_sixtydig;
		rgb[0] = 255 * (val);
		rgb[1] = 0;
		rgb[2] = 255;
	}
	else if (angle >= pi_sixtydig * 5 && angle < pi_sixtydig * 6) {
		double val = (angle - pi_sixtydig * 5) / pi_sixtydig;
		rgb[0] = 255;
		rgb[1] = 0;
		rgb[2] = 255 * (1 - val);
	}


}

