#include "CameraInput.hpp"
#include "ros/console.h"
//#include <type_traits>

CameraInput::CameraInput() : input_front(1), input_bottom(2), input_top(0)
{
    if (!input_front.isOpened()) {
        ROS_INFO("Unable to open front camera...");
    }

    if (!input_bottom.isOpened()) {
        ROS_INFO("Unable to open bottom camera...");
    }

    if (!input_top.isOpened()) {
        ROS_INFO("Unable to open top camera...");
    }
}

bool CameraInput::update()
{
    if(!input_front.read(frame_front)) {
        return false;
    }
    if(!input_bottom.read(frame_bottom)) {
        return false;
    }
    if(!input_top.read(frame_top)) {
        return false;
    }
    return true;
}

const cv::Mat& CameraInput::getFrameFront() { return frame_front; }

const cv::Mat& CameraInput::getFrameBottom() { return frame_bottom; }

const cv::Mat& CameraInput::getFrameTop() { return frame_top; }