/*
 *  Display the foreground mask on a VFD
 *  By CSK (csk@live.com)
 *  www.csksoft.net
 */


#include "common/common.h"

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect_c.h>
#include <vector>

#include "oled12864.h"

// Arduino hooks
#include <Arduino.h>
#include <Servo/Servo.h> 
#include <trace.h>
#include <interrupt.h>
#include <sys/stat.h>


OLED12864 oledDrv;
Servo ptzX, ptzY; 

#define PTZ_SERVO_X  2
#define PTZ_SERVO_Y  3

const int ptzx_min_ticks = 450;
const int ptzx_max_ticks = 2000;

const int ptzy_min_ticks = 1000;
const int ptzy_max_ticks = 2400;

using namespace cv;
using namespace std;


_u8 oledfb[128*64];

int ptz_x_ticks = (ptzx_max_ticks + ptzx_min_ticks ) /2;
int ptz_y_ticks = (ptzy_max_ticks + ptzy_min_ticks ) /2;


bool                working_flag;
Locker              tracker_lock;

float               tracker_move_vec[2];

static void * tracker_proc(void * data) {
    
    while (working_flag) {
        float local_move_vec[2];
    
        tracker_lock.lock();
        memcpy(local_move_vec, tracker_move_vec, sizeof(tracker_move_vec));
        tracker_lock.unlock();
        
        _s8 ctrlSignal[2];

        if (fabs(local_move_vec[0])<1.0f && local_move_vec[0]!=0.0f) {
            local_move_vec[0] = fabs(local_move_vec[0])/local_move_vec[0];
        }    

        if (fabs(local_move_vec[1])<1.0f && local_move_vec[1]!=0.0f) {
            local_move_vec[1] = fabs(local_move_vec[1])/local_move_vec[1];
        }    


        ctrlSignal[0] = -local_move_vec[0];
        ctrlSignal[1] = local_move_vec[1];

        ptz_x_ticks += ctrlSignal[0];
        ptz_y_ticks += ctrlSignal[1];

        if (ptz_x_ticks<ptzx_min_ticks) ptz_x_ticks  = ptzx_min_ticks;
        if (ptz_y_ticks<ptzy_min_ticks) ptz_y_ticks  = ptzy_min_ticks;  

        if (ptz_x_ticks>ptzx_max_ticks) ptz_x_ticks  = ptzx_max_ticks;
        if (ptz_y_ticks>ptzy_max_ticks) ptz_y_ticks  = ptzy_max_ticks;  

        ptzX.write(ptz_x_ticks);
        ptzY.write(ptz_y_ticks);

#if 0
        //printf("ctrl %d %d\n", ctrlSignal[0], ctrlSignal[1]);
        ptz_serialchn.senddata((_u8 *)ctrlSignal, sizeof(ctrlSignal));
        ptz_serialchn.waitforsent();
#else

#endif
        usleep(1000/20*1000);
    }
    return NULL;
}

static void fullscaleImgToOLED(Mat & input) {
    // clip the frame to 128x64
    
    Mat_<uchar>& rawgray = (Mat_<uchar>&)input;
    int pos = 0;
    for (int row= (input.rows-64)/2; row <  (input.rows-64)/2+64; ++row) {
        for (int col = 0; col < 128; ++col) {
            _u8 current = rawgray(row, col);
            oledfb[pos++] = current;
        }
    }

    // send to the oled
    oledDrv.fillData8bit(oledfb);
}

const char * cascadeName = "data/haarcascades/haarcascade_frontalface_alt.xml";
const char * nestedCascadeName = "data/haarcascades/haarcascade_eye_tree_eyeglasses.xml";

CascadeClassifier cascade, nestedCascade;
const double scale = 4.5;



static bool processingClassifier() {
    if (!nestedCascade.load(nestedCascadeName)) {
        printf("Cannot load the nestedCascade %s.\n", nestedCascadeName);
        return false;
    }

    if (!cascade.load(cascadeName)) {
        printf("Cannot load the cascadeName %s. \n", cascadeName);
        return false;
    }
    return true;
}


static void onAdjustPTZ(Point &center, int r) {
    

    if (center.x>200) center.x=200;
    if (center.x<-200) center.x=-200;

    if (center.y>200) center.y=200;
    if (center.y<-200) center.y=-200;

    float scale = 3.0f * (20.0f / r) * (20.0f / r);

    
    tracker_lock.lock();
    tracker_move_vec[0] = center.x/scale;
    tracker_move_vec[1] = center.y/scale;
    tracker_lock.unlock();

}

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
    int camera_id = 0;

    if (initArduino(argc, argv)) return -1;

    if (!processingClassifier()) return -2;

    
    cv::VideoCapture cap(camera_id);

    if(!cap.isOpened())  // check if we succeeded
    {
        printf("failed to open the camera with id %d.\n", camera_id);
        return -1;
    }
    
    
    oledDrv.init();
    ptzX.attach(PTZ_SERVO_X);
    ptzY.attach(PTZ_SERVO_Y);

    ptzX.write(ptz_x_ticks);
    ptzY.write(ptz_y_ticks);

    // start the tracking thread
    pthread_t thrd_tracker;
    working_flag = true;
    pthread_create(&thrd_tracker, NULL, tracker_proc, NULL);

    cv::Mat srcframe;
    cv::Mat destFrameRGB, destFrameGray_tmp;
    cv::Mat dispFrame, dispFrame_plot;
    
    cap >> srcframe; // get a new frame from camera

    int srcWidth = srcframe.cols;


    const int targetHeight = srcframe.rows / scale;
    const int targetWidth  = srcframe.cols / scale;
    
    destFrameRGB.create(targetHeight,targetWidth,srcframe.type());
    destFrameGray_tmp.create(targetHeight,targetWidth,CV_8U);


    const float scale2 = targetWidth/128.0f;

    const int dispFrameHeight = targetHeight/scale2;
    dispFrame.create(dispFrameHeight,128, CV_8U);
    dispFrame_plot.create(dispFrameHeight,128, CV_8U);

    char msgbuf[100];
    vector<Rect> faces;
    Point last_moving_vector;
    
    while (1) {
        faces.clear();
        cap >> srcframe; // get a new frame from camera

        // resize the frame
        resize(srcframe, destFrameRGB, cvSize(targetWidth, targetHeight), INTER_LINEAR );

        // convert to gray scale
        cvtColor(destFrameRGB, destFrameGray_tmp, CV_RGB2GRAY);

        equalizeHist( destFrameGray_tmp, destFrameGray_tmp );
   

        cascade.detectMultiScale( destFrameGray_tmp, faces,
                1.2, 2, 0
                |CV_HAAR_FIND_BIGGEST_OBJECT
                |CV_HAAR_DO_ROUGH_SEARCH
                |CV_HAAR_DO_CANNY_PRUNING 
                //| CV_HAAR_DO_ROUGH_SEARCH
                ,
                Size(20, 20) );
                

        rectangle(dispFrame_plot, cvRect(0, 0, dispFrame.cols, dispFrame.rows), cvScalar(0,0,0,0),CV_FILLED );
        resize(destFrameGray_tmp, dispFrame, cvSize(128,dispFrameHeight), INTER_LINEAR );
    //    cv::threshold(dispFrame,dispFrame,60,255,CV_THRESH_BINARY_INV);
           Canny( dispFrame, dispFrame, 60, 120, 3 );

        int i = 0;

            
        for (vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++, i++ ) {
            Point center;
            int   radius;
            center.x = (r->x + r->width*0.5) / scale2;
            center.y = (r->y + r->height*0.5) / scale2;
            
            radius = (r->width + r->height) * 0.25 / scale2;
            circle (dispFrame_plot, center, radius, cvScalar(255,255,255,255),1.5);
            cv::line(dispFrame_plot, cvPoint(center.x-radius-2, center.y), cvPoint(center.x+radius+2, center.y), cvScalar(255,255,255,255),1);
            cv::line(dispFrame_plot, cvPoint(center.x, center.y-radius-2), cvPoint(center.x, center.y+radius+2), cvScalar(255,255,255,255),1);

            center.x-=128/2;
            center.y-=dispFrame.rows/2;
            onAdjustPTZ(center, radius);
            last_moving_vector = center;
        } 

        if (faces.size()==0 ) {
            last_moving_vector.x /= 1.5;
            last_moving_vector.y /= 1.5;
            
            if (fabs(last_moving_vector.x) < 0.01) last_moving_vector.x = 0;
            if (fabs(last_moving_vector.y) < 0.01) last_moving_vector.y = 0;

            onAdjustPTZ(last_moving_vector, 1);
            if (!last_moving_vector.x && !last_moving_vector.y) {
                rectangle(dispFrame, cvRect(0, 0, dispFrame.cols, dispFrame.rows), cvScalar(0,0,0,0),CV_FILLED );
                cv::putText(dispFrame_plot,"Where are you?", cvPoint(1, dispFrame.rows/2),FONT_HERSHEY_SIMPLEX, 0.5, cvScalar(255,255,255,255));
            } else {
                cv::putText(dispFrame_plot,"I am finding..", cvPoint(1, dispFrame.rows/2),FONT_HERSHEY_SIMPLEX, 0.5, cvScalar(255,255,255,255));

            }
        }


        for (int row= (dispFrame_plot.rows-64)/2; row <  (dispFrame_plot.rows-64)/2+64; ++row) {
            for (int col = 0; col < 128; ++col) {
                _u8 current = dispFrame_plot.at<_u8>(row, col);
                if (current) {
                    dispFrame.at<_u8>(row, col) = !dispFrame.at<_u8>(row, col);
                }
            }
        }
        fullscaleImgToOLED(dispFrame);
    
        //usleep(1*1000); // frame limit
    }
    
    working_flag = false;
    pthread_join(thrd_tracker, NULL);
    return 0;
}

