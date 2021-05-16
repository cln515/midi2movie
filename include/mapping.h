#pragma once

class mapping{
public:
    mapping();

    void getpos(int note,int track,double* posinfo);

private:
    int mode = 0;

};