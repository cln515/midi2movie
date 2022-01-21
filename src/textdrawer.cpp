#include <textdrawer.h>


bool textDrawer::init(std::string fontpath,int fontSize_, int lineWidth_) {
	FT_Init_FreeType(&library);
	FT_Error fterror = FT_New_Face(library, fontpath.c_str() , 0, &face);

	if (fterror == FT_Err_Unknown_File_Format) {
		std::cerr << "[ERROR] Font file format is not supported!! " << std::endl; return false;
	}
	else if (fterror) {
		std::cerr << "[ERROR] Font file not found or it is broken! " << std::endl; return false;
	}
	fterror = FT_Select_Charmap(
		face,               // target face object
		FT_ENCODING_UNICODE // エンコード指定
	);
	if (fterror == FT_Err_Unknown_File_Format)
		return -1;
	else if (fterror)
		return -1;

	fterror = FT_Set_Pixel_Sizes(face, 0, fontSize_);
	fontSize = fontSize_;
	lineWidth = 10;
	//FT_F26Dot6 fontsize = 16 * 64;
	//fterror = FT_Set_Char_Size(
	//	face,                // handle to face object
	//	0,                   // char_width in 1/64th of points
	//	fontsize,            // char_height in 1/64th of points
	//	DPI,     // horizontal device resolution
	//	DPI);    // vertical device resolution
}


void textDrawer::draw(cv::Mat inoutImg,std::string targettext ,cv::Rect roi,uchar* rgb) {

	//std::string str = "こんにちは、世界！";
	//int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	//std::wstring u32str(size_needed, 0);
	//MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &u32str[0], size_needed);

	unsigned char* imgArray = inoutImg.data;
	int imageWidth = inoutImg.size().width;
	int imageHeight = inoutImg.size().height;

	FT_Error fterror;
	wchar_t str2[512];
	MultiByteToWideChar(CP_UTF8, 0, targettext.c_str(), strlen(targettext.c_str()) + 1, str2, MAX_PATH);

	std::wstring u32str(str2);

	int pen_x = roi.x;
	int pen_y = roi.y + fontSize;
	for (size_t k = 0; k < u32str.size(); k++) {

		if (u32str[k] == '\n') {
			pen_x = roi.x;
			pen_y += fontSize + lineWidth; // not useful for now
			continue;
		}
		FT_ULong character = u32str[k];
		FT_UInt char_index = FT_Get_Char_Index(face, character);

		fterror = FT_Load_Glyph(face, char_index, FT_LOAD_RENDER);
		if (fterror)
			return; // ignore errors

		FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

		int startx = pen_x +face->glyph->bitmap_left;
		int starty = pen_y -face->glyph->bitmap_top;


		int Width = face->glyph->bitmap.width;
		int Height = face->glyph->bitmap.rows;

		for (size_t y = 0; y < Height; y++) {
			for (size_t x = 0; x < Width; x++) {

				int xx = startx + x;
				int yy = starty + y;

				if (xx < 0 || yy < 0)continue;
				if (xx >= imageWidth || yy >= imageHeight)continue;
				//if (xx < roi.x || yy < roi.y)continue;
				//if (xx >= roi.x+roi.width || yy >= roi.y + roi.height)continue;

				if (face->glyph->bitmap.buffer[y*Width + x]) {
					imgArray[(yy*imageWidth + xx) * 3] = rgb[0];
					imgArray[(yy*imageWidth + xx) * 3 + 1] = rgb[1];
					imgArray[(yy*imageWidth + xx) * 3 + 2] = rgb[2];
				}else if (setBG) {
					uchar ch1 = (1-bgalpha)*imgArray[(yy*imageWidth + xx) * 3]+ (bgalpha)*bgColor[0];
					uchar ch2 = (1 - bgalpha)*imgArray[(yy*imageWidth + xx) * 3+1] + (bgalpha)*bgColor[1];
					uchar ch3 = (1 - bgalpha)*imgArray[(yy*imageWidth + xx) * 3+2] + (bgalpha)*bgColor[2];
					imgArray[(yy*imageWidth + xx) * 3] = ch1;
					imgArray[(yy*imageWidth + xx) * 3 + 1] = ch2;
					imgArray[(yy*imageWidth + xx) * 3 + 2] = ch3;
				}
			}
		}
		pen_x += face->glyph->advance.x >> 6;
	}


}
