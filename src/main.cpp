#define _USE_MATH_DEFINES
#include <iostream>
#include <MidiFile.h>
#include <scoredrawer.h>
#include <opencv2/opencv.hpp>
#include <Windows.h>
#include <gl\GL.h>
#include <gl\GLU.h>
#include "Eigen/Eigen"
#include "Eigen/Core"
#include <sstream>
#include <nlohmann/json.hpp>

#include <q/support/literals.hpp>
#include <q_io/audio_stream.hpp>
#include <q_io/audio_file.hpp>
#include <q/fft/fft.hpp>

#include <stdio.h>
#include <math.h>
#include <codecvt>
#include <string>
#include <iostream>
#include <locale>
#include <textdrawer.h>

int main(int argc, char* argv[]) {
	std::cout << "Kitty on your lap!" << std::endl;
	//load config file
	std::ifstream ifs(argv[1]);
	nlohmann::json config;
	ifs >> config;
	ifs.close();
	

	int vWidth = config["width"].get<int>();
	int vHeight = config["height"].get<int>();
	double duration = 0.0;
	scoreVis sv;
	//load spectol parameter

	std::string wavPath = config["wav"].get<std::string>();


	cycfi::q::wav_reader wav{ wavPath };
	bool bMono = (wav.num_channels() == 1);
	int ch = wav.num_channels();
	constexpr std::size_t p = 11;
	constexpr std::size_t n = 1 << p;

	float* buf;
	float* wavbuf;
	int wavbuf_length = wav.length() / ch;
	int minfreq_idx, maxfreq_idx;
	{
		std::cout << "\nLength: " << wav.length();
		std::cout << "\nSample frequency: " << wav.sps() << " Hz";
		std::cout << "\nChannels: " << wav.num_channels() << std::endl;
		std::cout << "\nlength(s): " << wav.length()/ (float)(wav.sps() * wav.num_channels()) << std::endl;
		duration= wav.length() / (float)(wav.sps() * wav.num_channels());
		int bufnum = n * ch;
		buf = (float*)malloc(sizeof(float)*bufnum);

		std::size_t loaded;
		wavbuf = (float*)malloc(sizeof(float)*wav.length() / ch);
		int cnt = 0;

		while ((loaded = wav.read(buf, bufnum)) != 0) {
			for (int i = 0; i < loaded / ch; ++i)
			{
				if (bMono)wavbuf[cnt] = buf[i];
				else wavbuf[cnt] = (buf[i * 2] + buf[i * 2] + 1) / ch;
				cnt++;
			}
		}
		double minfreq = 60;
		double maxfreq = 4000;
		minfreq_idx = minfreq / (wav.sps() / n);
		maxfreq_idx = maxfreq / (wav.sps() / n);
	}


	bool bSpec = false;
	unsigned char spectolColor[] = { 255,255,255 };
	if (!config["spec"].is_null()) {
		if (!config["spec"]["color"].is_null()) {
			std::vector<int> specc = config["spec"]["color"].get<std::vector<int>>();
			spectolColor[0] = specc[0];
			spectolColor[1] = specc[1];
			spectolColor[2] = specc[2];
		}
		sv.setLineColor(spectolColor);
		bSpec = true;

		int mode = config["spec"]["mode"].get<int>();

		if (mode == 0) {
			sv.setSpecMapping(new straightMapping(0.1));
		}
		else if (mode == 1) {
			double zitv = 0.1;
			double diag = 1.0;
			double s_angle = 0.0;
			double reso = 1.0;
			sv.setSpecMapping(new circleMapping(zitv, diag, s_angle, reso));
		}
	}



	bool bMidiDraw = false;
	double midialpha = 1.0;

	//load midi parameter
	smf::MidiFile midifile;
	if (!config["midi"].is_null()) {
		nlohmann::json config_midi = config["midi"];

		bMidiDraw = true;
		std::string midiPath = config_midi["file"].get<std::string>();
		int mode = config_midi["mode"].get<int>();
		double speed = config_midi["speed"].is_null() ? 1.0 : config_midi["speed"].get<double>();
		
		midifile.read(midiPath);
		midifile.doTimeAnalysis();
		midifile.linkNotePairs();
		int minNote = 255;
		int maxNote = 0;
		for (int track = 0; track < midifile.getTrackCount(); ++track) {
			for (int event = 0; event < midifile[track].size(); ++event) {
				if (midifile[track][event].isNoteOn()) {
					int note = midifile[track][event][1];
					if (note < minNote) {
						minNote = note;
					}
					if (note > maxNote) {
						maxNote = note;
					}
				}
			}
		}


		std::cout << "duration: " << midifile.getFileDurationInSeconds() << std::endl;
		std::cout << "minNote: " << minNote << std::endl;
		std::cout << "maxNote: " << maxNote << std::endl;
		// duration = midifile.getFileDurationInSeconds();
		double center = (minNote + maxNote)*0.5*0.05;
		Eigen::Matrix3d R = Eigen::Matrix3d::Identity();
		if (!config_midi["view"].is_null()) {
			Eigen::Matrix3d Rx, Ry, Rz;
			nlohmann::json viewp = config_midi["view"].get<nlohmann::json>();
			Rx = Eigen::AngleAxisd(viewp["rx"].get<double>(), Eigen::Vector3d::UnitX());
			Ry = Eigen::AngleAxisd(viewp["ry"].get<double>(), Eigen::Vector3d::UnitY());
			Rz = Eigen::AngleAxisd(viewp["rz"].get<double>(), Eigen::Vector3d::UnitZ());
			R = Rz * Ry* Rx;
		}
		if (!config_midi["alpha"].is_null()) {
			midialpha = config_midi["alpha"].get<double>();
		}

		if (mode == 0) {
			double xpos = config_midi["x"].is_null() ? 0.0 : config_midi["x"].get<double>();
			double ypos = config_midi["y"].is_null() ? center : center + config_midi["y"].get<double>();
			double zpos = config_midi["z"].is_null() ? -1.0 : config_midi["z"].get<double>();
			double zitv = config_midi["interval"].is_null() ? 0.1 : config_midi["interval"].get<double>();
			sv.setMapping(new straightMapping(zitv));
			sv.setRotation(R);
			sv.setXYZ(xpos, ypos, zpos);
		}
		else if (mode == 1) {
			double xpos = config_midi["x"].is_null() ? -1.0 : config_midi["x"].get<double>();
			double ypos = config_midi["y"].is_null() ? 0.0 : config_midi["y"].get<double>();
			double zpos = config_midi["z"].is_null() ? 0.0 : config_midi["z"].get<double>();
			double zitv = config_midi["interval"].is_null() ? 0.1 : config_midi["interval"].get<double>();
			double diag = config_midi["radius"].is_null() ? 1.0 : config_midi["radius"].get<double>();
			double s_angle = config_midi["startangle"].is_null() ? 0.0 : config_midi["startangle"].get<double>();
			double reso = config_midi["resolution"].is_null() ? 2 * M_PI / 127 : config_midi["resolution"].get<double>();

			Eigen::Matrix3d Rdef;
			Rdef = Eigen::AngleAxisd(M_PI / 2.0, Eigen::Vector3d::UnitY());
			sv.setMapping(new circleMapping(zitv, diag, s_angle, reso));
			sv.setRotation(Rdef*R);
			sv.setXYZ(xpos, ypos, zpos);

		}
		sv.setSpeed(speed);
	}

	//context creation
	{
		sv.dur = duration;
		sv.createContext(vWidth, vHeight);
	}




	std::string outputPath = config["output"].get<std::string>();
	
	std::vector<std::string> encoding = config["encode_target"].get<std::vector<std::string>>();
	if (encoding.size() == 0) {
		std::cout << "ERROR: encode_target required!!" << std::endl;
		return -1;
	}
	bool vis = true;
	if (!config["vis"].is_null()) {
		vis = config["vis"].get<bool>();
	}

	//back ground =======================
	bool setBG = false;

	std::vector<nlohmann::json> sources;
	if (!config["background"].is_null()) {//background image loading
		sources = config["background"]["source"].get<std::vector<nlohmann::json>>();
		setBG = true;
	}

	std::vector<cv::Mat> bg(sources.size());
	std::vector <double> dulations_bg(sources.size());
	std::vector <double> transition_duration(sources.size());
	int bg_idx = 0;
	if (setBG) {//background image loading
		for (int i = 0; i < sources.size(); i++) {
			
			cv::resize(cv::imread(sources.at(i)["file"]), bg[i], cv::Size(vWidth, vHeight)); ;
			if (!sources.at(i)["dulation"].is_null()) {
				dulations_bg[i]=(sources.at(i)["dulation"].get<double>());
			}
			else {
				dulations_bg[i] = (-1);
			}
			if (!sources.at(i)["transition"].is_null()) {
				transition_duration[i] = (sources.at(i)["transition"].get<double>());
			}
			else {
				transition_duration[i] = (1.0);
			}
		}
	}

	//text
	//std::vector<nlohmann::json> txt_sources,disp_txts;
	nlohmann::json textdata;
	int txtinx = 0;
	bool settxt=false;
	textDrawer td;
	std::string fontpath;
	int fontSize=20;
	int lineWidth = 10;
	int textx=0, texty=0;
	uchar colorbgr[3] = {255,255,255};
	uchar textbgbgr[3] = { 0,0,0 };
	double timeoffset = 0.0;
	if (!config["text"].is_null()) {//background image loading
		std::ifstream ifs_ts(config["text"].get<std::string>());
		ifs_ts >> textdata;
		ifs_ts.close();

		fontpath = textdata["font"].get<std::string>();
		if (!textdata["fontsize"].is_null())fontSize = textdata["fontsize"].get<int>();
		if (!textdata["line_interval"].is_null())lineWidth = textdata["line_interval"].get<int>();
		settxt = true;
		textx = textdata["x"];
		texty = textdata["y"];
		if (!textdata["color"].is_null()) {
			std::vector<int> c = textdata["color"].get<std::vector<int>>();
			colorbgr[0] = c.at(0);
			colorbgr[1] = c.at(1);
			colorbgr[2] = c.at(2);
		}
		if (!textdata["back"].is_null()) {
			double alpha = 1.0;
			if (!textdata["back"]["color"].is_null()) {
				std::vector<int> c = textdata["back"]["color"].get<std::vector<int>>();
				textbgbgr[0] = c.at(0);
				textbgbgr[1] = c.at(1);
				textbgbgr[2] = c.at(2);
			}if (!textdata["back"]["alpha"].is_null()) {
				alpha = textdata["back"]["alpha"].get<double>();
			}
			td.setBackDrawing(colorbgr, 1.0);
		}
		if(!textdata["timeoffset"].is_null()) {
			timeoffset = textdata["timeoffset"].get<double>();
		}
		if (!td.init(fontpath, fontSize, lineWidth))return -1;
	}

	/*{
		cv::Mat image_cv= cv::Mat::zeros(300, 500, CV_8UC3);
		cv::Rect ROI(20,20,300,500);
		uchar rgb[] = { 255,255,255 };
		td.draw(image_cv, ROI,rgb);
		cv::imshow("output", image_cv);
		cv::waitKey(0);
		return 0;
	}*/

	//start rendering======================
	GLubyte* buffer;
	Eigen::Vector3d t;

	cv::VideoWriter writer;
	writer.open("temp.mp4", cv::VideoWriter::fourcc('m', 'p', '4', 'v'), 30, cv::Size(vWidth, vHeight));
	double fftbuffer[2 * n];
	for (double i = 0; i < duration; i += 1/30.0) {
		sv.setView();
		if (bSpec) {
			double time = i;
			int bufidx = time * wav.sps();
			for (int j = 0; j < n * 2; j++) {
				if (j + bufidx >= wavbuf_length) {
					fftbuffer[j] = wavbuf[wavbuf_length-1] ;
				}
				else fftbuffer[j] = wavbuf[j + bufidx];
			}
			cycfi::q::fft<n>(fftbuffer);
			sv.setSpectrum(fftbuffer,n,minfreq_idx,maxfreq_idx);
			sv.render_spectrum();
		}

		cv::Mat colorimage = cv::Mat(cv::Size(vWidth, vHeight), CV_8UC3, cv::Scalar(0));

		if (bMidiDraw) {
			sv.render(midifile, i);
		}
		sv.bufferCopy(buffer);
		memcpy(colorimage.data, buffer, sizeof(uchar) * 3 * colorimage.size().width*colorimage.size().height);
		cv::flip(colorimage, colorimage, 0);
		//Background image blending
		if (setBG) {
			uchar* ci_ptr = colorimage.data;
			uchar* bg_ptr = bg.at(bg_idx).data;
			uchar* bg_ptr_next = bg.at(bg_idx).data; //bg.at(bg_idx).data;
			double blend_rate = 1.0;
			double trans_dur = transition_duration.at(bg_idx);
			if (dulations_bg.at(bg_idx) != -1 && dulations_bg.at(bg_idx) - i < trans_dur) {
				bg_ptr_next = bg.at(bg_idx+1).data;
				blend_rate = (dulations_bg.at(bg_idx) - i)/ trans_dur;
				if (blend_rate < 0)blend_rate = 0;
//				if (blend_rate > 1)blend_rate = 1;
			}

			int areasize = colorimage.size().area();
			for (int pix = 0; pix < areasize; pix++) {
				if (ci_ptr[pix * 3] == 0 && ci_ptr[pix * 3 + 1] == 0 && ci_ptr[pix * 3 + 2] == 0) {
					ci_ptr[pix * 3] = blend_rate*bg_ptr[pix * 3] + 
						(1.0- blend_rate) * bg_ptr_next[pix * 3];
					ci_ptr[pix * 3 + 1] = blend_rate * bg_ptr[pix * 3 + 1] +
						(1.0 - blend_rate) * bg_ptr_next[pix * 3 + 1];
					ci_ptr[pix * 3 + 2] = blend_rate * bg_ptr[pix * 3 + 2] +
						(1.0 - blend_rate) * bg_ptr_next[pix * 3 + 2];
				}
				else if (midialpha < 1.0) {
					ci_ptr[pix * 3] = (1-midialpha)*(blend_rate * bg_ptr[pix * 3] +
						(1.0 - blend_rate) * bg_ptr_next[pix * 3]) + midialpha * ci_ptr[pix * 3];
					ci_ptr[pix * 3 + 1] = (1 - midialpha)*(blend_rate * bg_ptr[pix * 3 + 1] +
						(1.0 - blend_rate) * bg_ptr_next[pix * 3 + 1])+ midialpha * ci_ptr[pix * 3+1];
					ci_ptr[pix * 3 + 2] = (1 - midialpha)*(blend_rate * bg_ptr[pix * 3 + 2] +
						(1.0 - blend_rate) * bg_ptr_next[pix * 3 + 2])+ midialpha * ci_ptr[pix * 3 + 2];
				}
			}

			if (dulations_bg.at(bg_idx) != -1 && dulations_bg.at(bg_idx) - i < 0.0) {
				bg_idx++;
			}
		}
		if(settxt && txtinx < textdata["data"].size()) {
			if (!textdata["data"][txtinx]["end"].is_null()) {
				while (textdata["data"][txtinx]["end"].get<double>() + timeoffset < i) {
					txtinx++;
					if (txtinx >= textdata["data"].size() || textdata["data"][txtinx]["end"].is_null())break;
				}
			}
			if (txtinx < textdata["data"].size()) {
				std::string str = textdata["data"][txtinx]["string"].get<std::string>();
				wchar_t str2[512];
				char str3[512];

				MultiByteToWideChar(CP_UTF8, 0, str.c_str(), strlen(str.c_str()) + 1, str2, MAX_PATH);
				WideCharToMultiByte(CP_ACP, 0, str2, sizeof(str2) / sizeof(str2[0]), str3, sizeof(str3), NULL, NULL);

				td.draw(colorimage, str,cv::Rect(textx,texty,100,100),colorbgr);
			}

		}





		if (vis) {
			cv::imshow("vis", colorimage); cv::waitKey(1);
		}

		writer << colorimage;
		free(buffer);
	}
	writer.release();




	if (std::find(encoding.begin(), encoding.end(), "twitter") != encoding.end())
	{
		std::stringstream ss;
		ss << "ffmpeg -y -i temp.mp4 -i " << wavPath << " -pix_fmt yuv420p -c:v libx264 -c:a aac " << outputPath << "_twitter.mp4";
		std::system(ss.str().c_str());
	}
	if (std::find(encoding.begin(), encoding.end(), "youtube") != encoding.end())
	{
		std::stringstream ss;
		ss << "ffmpeg -y -i temp.mp4 -i " << wavPath << " -ar 48000 -ac 2 -pix_fmt yuv420p -movflags +faststart -c:v libx264 -c:a aac -profile:a aac_low -b:v 15M -b:a 384k -coder 1 -profile:v high -vf yadif=0:-1:1 -bf 2 " << outputPath << "_youtube.mp4";
		std::system(ss.str().c_str());
	}
	//if (std::find(encoding.begin(), encoding.end(), "instagram") != encoding.end())
	//{
	//	std::stringstream ss;
	//	ss << "ffmpeg -y -i temp.mp4 -i " << wavPath << " -pix_fmt yuv420p -c:v libx264 -c:a aac -b:v 3500 -b:a 128k " << outputPath << "_instagram.mp4";
	//	std::system(ss.str().c_str());
	//}
	std::remove("temp.mp4");
	return 0;
}







