#pragma once

#include <iostream>
#include <MidiFile.h>
#include <Windows.h>
#include <gl\GL.h>
#include <gl\GLU.h>
#include "Eigen/Eigen"
#include "Eigen/Core"
#include <mapping.h>

class scoreVis{
public:
    scoreVis();
    void render(smf::MidiFile midifile, GLubyte*& colorImage,Eigen::Matrix3d R, Eigen::Vector3d t);
    void createContext(int view_w, int view_h);
    mapping m;
}