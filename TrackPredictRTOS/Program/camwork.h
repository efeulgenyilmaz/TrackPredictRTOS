//camwork.h

#ifndef CAMWORK_H
#define CAMWORK_H

#include <opencv2/opencv.hpp>
#include "dtst.h"

extern int time_mode;
extern int camera_status;

extern int width;
extern int height;

bool no_object;

typedef struct {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} RGBPixel;

int isRedPixel(RGBPixel pixel);
node * calculateRedCentroid(/*RGBPixel* image, */int width, int height, node* head, bool* no_object);
int check_args(int argc, char *argv[]);
void keep_the_camera_open();
cv::Mat capture_frame(cv::VideoCapture& cap);
//RGBPixel* malloc_and_format();
void test();

#endif
