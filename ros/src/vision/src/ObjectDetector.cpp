#include "ObjectDetector.hpp"
#include "Distance.hpp"

#include "opencv2/opencv.hpp"
#include "opencv2/xfeatures2d.hpp"
#include "ros/console.h"
#include <stdio.h>

ObjectDetector::ObjectDetector(CameraInput& input, std::string object_type) : camera_input(input)
{
    /* IMPORTANT: Must use fully qualified file path for ROS as relative file paths
       navigate from $ROS_HOME */
    if(object_type == "Jiangshi")
        detector.object_img = cv::imread("../images/Jiangshi_lg.png", cv::IMREAD_GRAYSCALE);
    else if(object_type == "Aswang")
        detector.object_img = cv::imread("../images/Aswang_lg.png", cv::IMREAD_GRAYSCALE);
    else if(object_type == "Draugr")
        detector.object_img = cv::imread("../images/Draugr_lg.png", cv::IMREAD_GRAYSCALE);
    else if(object_type == "Vetalas")
        detector.object_img = cv::imread("../images/Vetalas_lg.png", cv::IMREAD_GRAYSCALE);

    if (detector.object_img.data == NULL) {
        ROS_INFO("Unable to process image: Either image was not found, or image is empty");
        return;
    }

    detector.sift = cv::xfeatures2d::SIFT::create();
    detector.matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::FLANNBASED);
    detector.sift->detectAndCompute(detector.object_img, cv::noArray(), detector.keypoints1, detector.descriptors1);
}

ObjectDetector::~ObjectDetector(){}


bool ObjectDetector::update()
{
    found_object = false;
    cv::Mat gray_frame;

    // Convert input.getFrameFront() to black and white
    cv::cvtColor(camera_input.getFrameFront(), gray_frame, cv::COLOR_BGR2GRAY);

    detector.sift->detectAndCompute(gray_frame, cv::noArray(), detector.keypoints2, detector.descriptors2);

    // If no descriptors found, then return false, no use further in method
    if (detector.descriptors2.empty()) {
        return found_object;
    }

    // matching the descriptor vectors with a FLANN based matcher
    std::vector< std::vector<cv::DMatch> > knn_matches;
    detector.matcher->knnMatch(detector.descriptors1, detector.descriptors2, knn_matches, 2);

    // Filter matches using the Lowe's ratio test
    std::vector<cv::DMatch> good_matches;
    for (size_t i = 0; i < knn_matches.size(); i++){
        if (knn_matches[i][0].distance < ratio_thresh * knn_matches[i][1].distance){
            good_matches.push_back(knn_matches[i][0]);
        }
    }
    cv::Mat img_matches;
    if(good_matches.size() >= min_match_count){
        std::vector<cv::Point2f> src_pts;
        std::vector<cv::Point2f> dst_pts;

        for(cv::DMatch x : good_matches){
            src_pts.push_back(detector.keypoints1[x.queryIdx].pt);
            dst_pts.push_back(detector.keypoints2[x.trainIdx].pt);
        }
        cv::Mat H = cv::findHomography(src_pts, dst_pts, cv::RANSAC);

        // get the corners from the jiangshi image
        std::vector<cv::Point2f> obj_corners(4);
        obj_corners[0] = cvPoint(0,0); obj_corners[1] = cvPoint( detector.object_img.cols, 0 );
        obj_corners[2] = cvPoint(detector.object_img.cols, detector.object_img.rows);
        obj_corners[3] = cvPoint(0, detector.object_img.rows);
        std::vector<cv::Point2f> scene_corners(4);

        if (!H.empty()){
            cv::perspectiveTransform(obj_corners, scene_corners, H);
            cv::Rect rect(scene_corners[0], scene_corners[2]);
            object_rect = rect;
            found_object = true;

            distance_x_front = Distance::getDistanceX(object_rect, object_width, camera_input.getFrameFront());
            distance_y_front = Distance::getDistanceY(object_rect, object_height, camera_input.getFrameFront());
            distance_z_front = Distance::getDistanceZ(object_rect, object_width, 1);
        }
    }
    return found_object;
}
