#define _USE_MATH_DEFINES
#include <iostream>
#include <MidiFile.h>
#include <opencv2/opencv.hpp>
#include <Windows.h>
#include <gl\GL.h>
#include <gl\GLU.h>
#include "Eigen/Eigen"
#include "Eigen/Core"
#include <sstream>

void createContext(int view_w, int view_h);

void render(smf::MidiFile midifile, GLubyte*& colorImage, Eigen::Matrix3d R, Eigen::Vector3d t);
void InitPers(int viewWidth, int viewHeight, double znear, double depthResolution, double* intrinsic);
int viewWidth_ = 1280;
int viewHeight_ = 720;
double znear=0.1; 
double depthResolution=20.0;
double intrinsic[] = { 640, 360 ,200, 200 };
void HSVAngle2Color(double radangle, unsigned char* rgb);

void trackColor(int track, unsigned char* rgb, unsigned char* rgb2) {
	HSVAngle2Color(track*1.0, rgb);
	HSVAngle2Color(track*1.0+0.5, rgb2);
}

int main(int argv, char* argc[]) {
	std::cout << "Kitty on your lap!" << std::endl;

	smf::MidiFile midifile;
	midifile.read(argc[1]);//file.mid
	midifile.doTimeAnalysis();
	midifile.linkNotePairs();
	int minNote = 255;
	int maxNote = 0;
	for (int track = 0; track < midifile.getTrackCount(); ++track) {

		for (int event = 0; event < midifile[track].size(); ++event) {
			if (midifile[track][event].isNoteOn()) {
				std::cout << midifile[track][event].seconds << std::endl;
				int note = midifile[track][event][1];
				if (note < minNote) {
					minNote = note;
				}
				if (note > maxNote) {
					maxNote = note;
				}
			}
		}

		std::cout << std::endl;
	}
	
	
	std::cout << "duration: " << midifile.getFileDurationInSeconds() << std::endl;
	std::cout << "minNote: " << minNote << std::endl;
	std::cout << "maxNote: " << maxNote << std::endl;
	double duration = midifile.getFileDurationInSeconds();
	createContext(viewWidth_, viewHeight_);
	GLubyte* buffer;
	Eigen::Vector3d t;
	Eigen::Matrix3d R = Eigen::Matrix3d::Identity();

	double center = (minNote + maxNote)*0.5*0.05;
	cv::VideoWriter writer;
	writer.open("temp.mp4", cv::VideoWriter::fourcc('m', 'p', '4', 'v'), 30, cv::Size(viewWidth_,viewHeight_));
	for (double i = 0; i < duration; i += 1/30.0) {
		t << i, center, -1;
		render(midifile, buffer, R, t);
		cv::Mat colorimage = cv::Mat(cv::Size(viewWidth_, viewHeight_), CV_8UC3);
		memcpy(colorimage.data, buffer, sizeof(uchar) * 3 * colorimage.size().width*colorimage.size().height);
		cv::flip(colorimage, colorimage, 0);
		cv::imshow("vis", colorimage); cv::waitKey(1);
		writer << colorimage;

		free(buffer);
	}
	writer.release();

	std::stringstream ss;
	ss << "ffmpeg -i temp.mp4 -i 0504.wav -pix_fmt yuv420p -c:v libx264 -c:a aac output2.mp4";
	std::system(ss.str().c_str());

	return 0;
}




void createContext(int view_w, int view_h) {
	PIXELFORMATDESCRIPTOR _pfd = {
sizeof(PIXELFORMATDESCRIPTOR),	//	Size of this struct
1,	//	Versin of this structure
PFD_DRAW_TO_BITMAP | PFD_SUPPORT_OPENGL |
PFD_GENERIC_ACCELERATED,
//		PFD_GENERIC_FORMAT,
		//	Pixel buffer flags
		PFD_TYPE_RGBA,	//	Type of pixel data
		24,	//	The number of color bitplanes
		0, 0, 0, 0, 0, 0,	//	Number of each color bitplanes and shift count
		0, 0,	//	Number of alpha bitplanes and shift count
		0, 0, 0, 0, 0,	//	Number of accumulation bits
		32,	//	Z depth
		0,	//	Stencil depth
		0,	//	Number of auxiliary buffers
		PFD_MAIN_PLANE,	//	Ignored
		0,	//	Reserved
		0,	//	Ignored
		0,	//	Transparent color value
		0,	//	Ignored
	};
	GLint view[4];
	HDC		_hdc_ = CreateCompatibleDC(NULL);
	//viewWidth_ = viewWidth_stat = view_w;
	//viewHeight_ = viewHeight_stat = view_h;



	DWORD m_DIBWidth = view_w;
	DWORD m_DIBHeight = view_h;
	DWORD m_BPP = 24;

	// Create BITMAPINFOHEADER
	BITMAPINFOHEADER* m_PBIH = new BITMAPINFOHEADER;
	int iSize = sizeof(BITMAPINFOHEADER);
	::memset(m_PBIH, 0, iSize);

	m_PBIH->biSize = sizeof(BITMAPINFOHEADER);
	m_PBIH->biWidth = m_DIBWidth;
	m_PBIH->biHeight = m_DIBHeight;
	m_PBIH->biPlanes = 1;
	m_PBIH->biBitCount = m_BPP;
	m_PBIH->biCompression = BI_RGB;

	// Create DIB
	void* m_PBits;
	HBITMAP m_hbitmap_old;
	HBITMAP m_hbitmap = ::CreateDIBSection(
		_hdc_,
		(BITMAPINFO*)m_PBIH, DIB_RGB_COLORS,
		&m_PBits, NULL, 0
	);

	m_hbitmap_old = (HBITMAP)::SelectObject(_hdc_, m_hbitmap);
	DWORD dwLength;
	if ((m_DIBWidth * 3) % 4 == 0) /* バッファの１ラインの長さを計算 */
		dwLength = m_DIBWidth * 3;
	else
		dwLength = m_DIBWidth * 3 + (4 - (m_DIBWidth * 3) % 4);

	int		_pfid = ChoosePixelFormat(_hdc_, &_pfd);
	::SetPixelFormat(_hdc_, _pfid, &_pfd);
	HGLRC	_hrc = ::wglCreateContext(_hdc_);
	::wglMakeCurrent(_hdc_, _hrc);
}

void render(smf::MidiFile midifile, GLubyte*& colorImage,Eigen::Matrix3d R, Eigen::Vector3d t) {
	GLint view[4];
	{
		InitPers(viewWidth_, viewHeight_, znear, depthResolution, intrinsic);
		glGetIntegerv(GL_VIEWPORT, view);

		glBegin(GL_TRIANGLES);
		unsigned char trackColors[3], trackColore[3];
		for (int track = 0; track < midifile.getTrackCount(); ++track) {
			trackColor(track, trackColors, trackColore);
			for (int event = 0; event < midifile[track].size(); ++event) {
				if (midifile[track][event].isNoteOn()) {
					double dx = midifile[track][event].seconds;
					double dy = midifile[track][event][1] * 0.05;
					double dz = track * 0.5;
					double length = midifile[track][event].getDurationInSeconds();
					Eigen::Vector3d v1, v2, v3, v4;
					v1 << dx, dy, dz;
					v2 << dx, dy+0.05, dz;
					v3 << dx + length, dy, dz;
					v4 << dx + length, dy+0.05, dz;

					v1 = R.transpose() * (v1 - t);
					v2 = R.transpose() * (v2 - t);
					v3 = R.transpose() * (v3 - t);
					v4 = R.transpose() * (v4 - t);


					glColor3ub(trackColors[0], trackColors[1], trackColors[2]);
					glVertex3f(v1(0), v1(1), v1(2));
					glColor3ub(trackColors[0], trackColors[1], trackColors[2]);
					glVertex3f(v2(0), v2(1), v2(2));
					glColor3ub(trackColore[0], trackColore[1], trackColore[2]);
					glVertex3f(v3(0), v3(1), v3(2));

					glColor3ub(trackColors[0], trackColors[1], trackColors[2]);
					glVertex3f(v2(0), v2(1), v2(2));
					glColor3ub(trackColore[0], trackColore[1], trackColore[2]);
					glVertex3f(v3(0), v3(1), v3(2));
					glColor3ub(trackColore[0], trackColore[1], trackColore[2]);
					glVertex3f(v4(0), v4(1), v4(2));


				}
			}
		}
		glEnd();
	}
	
	colorImage = (GLubyte*)malloc(sizeof(GLubyte)*view[2] * view[3] * 3);
	glReadPixels(view[0], view[1], view[2], view[3], GL_RGB, GL_UNSIGNED_BYTE, colorImage);


}


void InitPers(int viewWidth, int viewHeight, double znear, double depthResolution, double* intrinsic) {

	glViewport(0, 0, viewWidth, viewHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glClearColor(1.0, 1.0, 1.0, 1.0);
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

