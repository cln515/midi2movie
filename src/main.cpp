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

int main(int argc, char* argv[]) {
	std::cout << "Kitty on your lap!" << std::endl;
	//load config file
	std::ifstream ifs(argv[1]);
	nlohmann::json config;
	ifs >> config;
	ifs.close();
	

	bool bMidiDraw = false;
	int vWidth = config["width"].get<int>();
	int vHeight = config["height"].get<int>();


	//load midi parameter
	scoreVis sv;
	smf::MidiFile midifile;
	double duration = 0.0;
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
		 duration = midifile.getFileDurationInSeconds();
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
		sv.dur = duration;
		sv.createContext(vWidth, vHeight);
		sv.setSpeed(speed);

	}


	//load spectol parameter

	std::string wavPath = config["wav"].get<std::string>();
	
	bool bSpec = true;
	if (!config["spec"].is_null()) {
		bSpec = config["spec"].get<bool>();
	}
	cycfi::q::wav_reader wav{ wavPath };
	bool bMono = (wav.num_channels() == 1);
	int ch = wav.num_channels();
	constexpr std::size_t p = 11;
	constexpr std::size_t n = 1 << p;
	
	float* buf;
	float* wavbuf;
	int wavbuf_length = wav.length() / ch;
	int minfreq_idx,maxfreq_idx;
	if (bSpec) {
		std::cout << "\nLength: " << wav.length();
		std::cout << "\nSample frequency: " << wav.sps() << " Hz";
		std::cout << "\nChannels: " << wav.num_channels() << std::endl;
		
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
	bool setBG = false;

	std::vector<nlohmann::json> sources;
	if (!config["background"].is_null()) {//background image loading
		sources = config["background"]["source"].get<std::vector<nlohmann::json>>();
		setBG = true;
	}

	std::vector<cv::Mat> bg(sources.size());
	std::vector <double> dulations_bg(sources.size());
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
		}
	}






	
	GLubyte* buffer;
	Eigen::Vector3d t;

	cv::VideoWriter writer;
	writer.open("temp.mp4", cv::VideoWriter::fourcc('m', 'p', '4', 'v'), 30, cv::Size(vWidth, vHeight));
	double fftbuffer[2 * n];
	for (double i = 0; i < duration; i += 1/30.0) {
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
		}

		cv::Mat colorimage = cv::Mat(cv::Size(vWidth, vHeight), CV_8UC3, cv::Scalar(0));

		if (bMidiDraw) {
			sv.render(midifile, buffer, i);
			memcpy(colorimage.data, buffer, sizeof(uchar) * 3 * colorimage.size().width*colorimage.size().height);
			cv::flip(colorimage, colorimage, 0);
		}
		
		//Background image blending
		if (setBG) {
			uchar* ci_ptr = colorimage.data;
			uchar* bg_ptr = bg.at(bg_idx).data;
			uchar* bg_ptr_next = bg.at(bg_idx).data; //bg.at(bg_idx).data;
			double blend_rate = 1.0;
			if (dulations_bg.at(bg_idx) != -1 && dulations_bg.at(bg_idx) - i < 1.0) {
				bg_ptr_next = bg.at(bg_idx+1).data;
				blend_rate = dulations_bg.at(bg_idx) - i;
				if (blend_rate < 0)blend_rate = 0;
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
				//else: Todo alpha blending
			}

			if (dulations_bg.at(bg_idx) != -1 && dulations_bg.at(bg_idx) - i < 0.0) {
				bg_idx++;
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
	if (std::find(encoding.begin(), encoding.end(), "instagram") != encoding.end())
	{
		std::stringstream ss;
		ss << "ffmpeg -y -i temp.mp4 -i " << wavPath << " -pix_fmt yuv420p -c:v libx264 -c:a aac -b:v 3500 -b:a 128k " << outputPath << "_instagram.mp4";
		std::system(ss.str().c_str());
	}
	std::remove("temp.mp4");
	return 0;
}







