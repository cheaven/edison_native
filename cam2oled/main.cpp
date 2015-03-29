/*
 *  Opencv functionality validator
 *  By CSK
 *
 */

#include "common/common.h"

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc.hpp>

#include "oled12864.h"

// Arduino hooks
#include <Arduino.h>
#include <trace.h>
#include <interrupt.h>
#include <sys/stat.h>

OLED12864 oledDrv;
using namespace cv;


int initArduino(int argc, char * argv[] ) {

	char *platform_path = NULL;
	struct stat s;
	int err;


	// make ttyprintk at some point
	stdout = freopen("/tmp/log.txt", "w", stdout);
	if (stdout == NULL){
	    fprintf(stderr, "unable to remap stdout !\n");
	    exit(-1);
	}
	fflush(stdout);

	stderr = freopen("/tmp/log_er.txt", "w", stderr);
	if (stderr == NULL){
	    printf("Unable to remap stderr !\n");
	    exit(-1);
	}
	fflush(stderr);

	// Snapshot time counter
	if (timeInit() < 0)
		exit(-1);

	// debug for the user
	if (argc < 2){
		fprintf(stderr, "./sketch tty0\n");
		return -1;
	}
	fflush(stdout);

	// check if we're running on the correct platform
	// and refuse to run if no match

	// Call Arduino init
	init(argc, argv);

	// Init IRQ layer
	// Called after init() to ensure I/O permissions inherited by pthread
	interrupt_init();

    return 0;

}

int main( int argc, char * argv[] ) {

    if (initArduino(argc, argv)) return -1;


    // grab one frame from the camera specified via arg[1]
    int camera_id = 0;

    if (argc > 1) camera_id = atoi(argv[1]);

    cv::VideoCapture cap(camera_id);

    if(!cap.isOpened())  // check if we succeeded
    {
        printf("failed to open the camera with id %d.\n", camera_id);
        return -1;
    }
    
    cv::Mat srcframe;
    cv::Mat destFrameRGB, destFrameGray_tmp;
    _u8 oledfb[128*64];

    cap >> srcframe; // get a new frame from camera
    // convert the frame size to 128xN
    int srcWidth = srcframe.cols;

    float scale = srcWidth/128.0f;

    const int targetHeight = srcframe.rows / scale;
    const int targetWidth  = 128;
    
    printf("targetHeight = %d, targetWidth = %d\n", targetHeight,targetWidth);
    destFrameRGB.create(targetHeight,targetWidth,srcframe.type());
    destFrameGray_tmp.create(targetHeight,targetWidth,CV_8U);
    oledDrv.init();

    while (1) {
        // capture one frame

        cap >> srcframe; // get a new frame from camera

        // resize the frame
        resize(srcframe, destFrameRGB, cvSize(targetWidth, targetHeight));

        // convert to gray scale
        cvtColor(destFrameRGB, destFrameGray_tmp, CV_RGB2GRAY);

#if 1
      //  blur( destFrameGray_tmp, destFrameGray_tmp, Size(2,2) );
        Canny( destFrameGray_tmp, destFrameGray_tmp, 60, 120, 3 );
#else
                // threshold
        cv::threshold(destFrameGray_tmp,destFrameGray_tmp,128,255,CV_THRESH_BINARY);

#endif
        // clip the frame to 128x64

        Mat_<uchar>& rawgray = (Mat_<uchar>&)destFrameGray_tmp;
        int pos = 0;
        for (int row= (targetHeight-64)/2; row <  (targetHeight-64)/2+64; ++row) {
            for (int col = 0; col < 128; ++col) {
                _u8 current = rawgray(row, col);
                oledfb[pos++] = current;
            }
        }

        oledDrv.fillData8bit(oledfb);

        usleep(1*1000); // frame limit

    }

    return 0;
}