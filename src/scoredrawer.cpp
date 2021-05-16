#include "scoredrawer.h"

void scoreVis::createContext(int view_w, int view_h) {
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
	if ((m_DIBWidth * 3) % 4 == 0) /* �o�b�t�@�̂P���C���̒������v�Z */
		dwLength = m_DIBWidth * 3;
	else
		dwLength = m_DIBWidth * 3 + (4 - (m_DIBWidth * 3) % 4);

	int		_pfid = ChoosePixelFormat(_hdc_, &_pfd);
	::SetPixelFormat(_hdc_, _pfid, &_pfd);
	HGLRC	_hrc = ::wglCreateContext(_hdc_);
	::wglMakeCurrent(_hdc_, _hrc);
}


void scoreVis::render(smf::MidiFile midifile, GLubyte*& colorImage,Eigen::Matrix3d R, Eigen::Vector3d t) {
	GLint view[4];
	{
		InitPers(viewWidth_, viewHeight_, znear, depthResolution, intrinsic);
		glGetIntegerv(GL_VIEWPORT, view);

        //note drawing
		glBegin(GL_TRIANGLES);
		unsigned char trackColors[3], trackColore[3];
        double pos[4];
		for (int track = 0; track < midifile.getTrackCount(); ++track) {
			trackColor(track, trackColors, trackColore);
			for (int event = 0; event < midifile[track].size(); ++event) {
				if (midifile[track][event].isNoteOn()) {
					double dx = midifile[track][event].seconds;
					double dy = midifile[track][event][1] * 0.05;
					double dz = track * 0.5;
					double length = midifile[track][event].getDurationInSeconds();

                    m.getpos(midifile[track][event][1],track,pos);

					Eigen::Vector3d v1, v2, v3, v4;
					v1 << dx,                   pos[0], pos[1];
					v2 << dx,                   pos[2], pos[3];
					v3 << dx + length,          pos[0], pos[1];
					v4 << dx + length,          pos[2], pos[3];

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