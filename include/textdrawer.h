#include <iostream>
#include <opencv2/opencv.hpp>

#include <Windows.h>

#include <codecvt>
#include <string>
#include <iostream>
#include <locale>

#include <ft2build.h>
#include FT_FREETYPE_H

#define DPI   300


class textDrawer{
public:
	bool init(std::string fontpath,int fontSize_,int lineWidth_ = 10);
	void draw(cv::Mat inoutImg,std::string targetText, cv::Rect roi, uchar* rgb);
	void setBackDrawing(uchar* bgColor_,double bgalpha_) {
		setBG = true;
		bgColor = bgColor_;//pointer copy
		bgalpha = bgalpha_;
	}

	FT_Library library;
	FT_Face face;
	int fontSize;
	int lineWidth;
	bool setBG=false;
	uchar* bgColor;
	double bgalpha;
};