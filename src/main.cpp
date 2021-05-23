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


int main(int argc, char* argv[]) {
	std::cout << "Kitty on your lap!" << std::endl;

	std::ifstream ifs(argv[1]);
	nlohmann::json config;
	ifs >> config;
	ifs.close();
	
	std::string midiPath = config["midi"].get<std::string>();
	std::string wavPath = config["wav"].get<std::string>();
	std::string outputPath = config["output"].get<std::string>();
	int mode = config["mode"].get<int>();
	int vWidth = config["width"].get<int>();
	int vHeight = config["height"].get<int>();

	double speed = config["speed"].is_null() ?1.0 :config["speed"].get<double>();
	smf::MidiFile midifile;
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
	double duration = midifile.getFileDurationInSeconds();
	double center = (minNote + maxNote)*0.5*0.05;

	Eigen::Matrix3d R = Eigen::Matrix3d::Identity();
	if (!config["view"].is_null()) {
		Eigen::Matrix3d Rx, Ry, Rz;
		nlohmann::json viewp = config["view"].get<nlohmann::json>();
		Rx = Eigen::AngleAxisd(viewp["rx"].get<double>(), Eigen::Vector3d::UnitX());
		Ry = Eigen::AngleAxisd(viewp["ry"].get<double>(), Eigen::Vector3d::UnitY());
		Rz = Eigen::AngleAxisd(viewp["rz"].get<double>(), Eigen::Vector3d::UnitZ());
		R = Rz * Ry* Rx;
	}

	scoreVis sv;
	if (mode == 0) {
		double xpos = config["x"].is_null() ? 0.0 : config["x"].get<double>();
		double ypos = config["y"].is_null() ? center : center + config["y"].get<double>();
		double zpos = config["z"].is_null() ? -1.0 : config["z"].get<double>();
		double zitv = config["interval"].is_null() ? 0.1 : config["interval"].get<double>();
		sv.setMapping(new straightMapping(zitv));
		sv.setRotation(R);
		sv.setXYZ(xpos,ypos, zpos);
	}
	else if (mode == 1) {

		double xpos = config["x"].is_null() ? -1.0 : config["x"].get<double>();
		double ypos = config["y"].is_null() ? 0.0 : config["y"].get<double>();
		double zpos = config["z"].is_null() ? 0.0 : config["z"].get<double>();
		double zitv = config["interval"].is_null() ? 0.1 : config["interval"].get<double>();
		double diag = config["radius"].is_null() ? 1.0 : config["radius"].get<double>();
		double s_angle = config["startangle"].is_null() ? 0.0 : config["startangle"].get<double>();
		double reso = config["resolution"].is_null() ? 2*M_PI/127 : config["resolution"].get<double>();

		Eigen::Matrix3d Rdef;
		Rdef = Eigen::AngleAxisd(M_PI/2.0, Eigen::Vector3d::UnitY());
		sv.setMapping(new circleMapping(zitv, diag, s_angle, reso));
		sv.setRotation(Rdef*R);
		sv.setXYZ(xpos, ypos, zpos);
	}
	



	sv.dur = duration;
	sv.createContext(vWidth, vHeight);
	GLubyte* buffer;
	Eigen::Vector3d t;

	sv.setSpeed(speed);
	cv::VideoWriter writer;
	writer.open("temp.mp4", cv::VideoWriter::fourcc('m', 'p', '4', 'v'), 30, cv::Size(vWidth, vHeight));
	for (double i = 0; i < duration; i += 1/30.0) {
		sv.render(midifile, buffer, i);
		cv::Mat colorimage = cv::Mat(cv::Size(vWidth, vHeight), CV_8UC3);
		memcpy(colorimage.data, buffer, sizeof(uchar) * 3 * colorimage.size().width*colorimage.size().height);
		cv::flip(colorimage, colorimage, 0);
		cv::imshow("vis", colorimage); cv::waitKey(1);
		writer << colorimage;

		free(buffer);
	}
	writer.release();

	std::stringstream ss;
	ss << "ffmpeg -i temp.mp4 -i " << wavPath << " -pix_fmt yuv420p -c:v libx264 -c:a aac " << outputPath;
	std::system(ss.str().c_str());

	return 0;
}







