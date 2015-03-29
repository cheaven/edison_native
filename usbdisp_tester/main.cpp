/*
 *  Opencv functionality validator
 *  By CSK
 *
 */

#include "common/common.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>


#include <rp/infra_config.h>
#include <rp/deps/libusbx_wrap/libusbx_wrap.h>
#include <rp/drivers/display/rpusbdisp/rpusbdisp.h>
#include <rp/drivers/display/rpusbdisp/c_interface.h>

using namespace std;
using namespace rp::util;
using namespace rp::deps::libusbx_wrap;
using namespace rp::drivers::display;


shared_ptr<RoboPeakUsbDisplayDevice> display;


int main( int argc, char * argv[] ) {
    // grab one frame from the camera specified via arg[1]
    int camera_id = 0;

    if (argc > 1) camera_id = atoi(argv[1]);

    cv::VideoCapture cap(camera_id);

    if(!cap.isOpened())  // check if we succeeded
    {
        printf("failed to open the camera with id %d.\n", camera_id);
        return -1;
    }
    
    // capture one frame
    cv::Mat frame;

    cv::Mat scaled_qvga;
    cv::Mat RGB565_Frame;
    

    RGB565_Frame.create(240,320, CV_16UC1);
    scaled_qvga.create(240,320, CV_8UC3);
 
    
    display = RoboPeakUsbDisplayDevice::openFirstDevice();
    display->enable();
    
    while (1) {
        cap >> frame; // get a new frame from camera
        // scale to QVGA size...
        cv::resize(frame, scaled_qvga,cv::Size(320,240));
        // convert to BGR565 format...
        cv::cvtColor(scaled_qvga, RGB565_Frame,  CV_RGB2BGR565);
        // send to diplay
        display->bitblt(0,0,320,240,RoboPeakUsbDisplayBitOperationCopy,RGB565_Frame.ptr());

    }
    return 0;
}
