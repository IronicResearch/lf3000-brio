//#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <stdio.h>

int lowerH=61;
int lowerS=0;
int lowerV=77;

int upperH=97;
int upperS=256;
int upperV=256;
static const std::string videoName = "Video";
static const std::string lightName = "Light";
static const double kFrameWidth = 320;
static const double kFrameHeight = 240;
static const int waitDuration = 80; // in ms

//This function threshold the HSV image and create a binary image
cv::Mat GetThresholdedImage(const cv::Mat &imgHSV){
 
  cv::Mat imgThresh(imgHSV.size(), CV_8U, 1);
  cv::inRange(imgHSV, cv::Scalar(lowerH,lowerS,lowerV), cv::Scalar(upperH,upperS,upperV), imgThresh); 
 
 return imgThresh;
}

//This function create two windows and 6 trackbars for the light window
void SetwindowSettings(){
  cv::namedWindow(videoName.c_str());
  cv::namedWindow(lightName.c_str());
 
  cv::createTrackbar("LowerH", lightName.c_str(), &lowerH, 180, NULL);
  cv::createTrackbar("UpperH", lightName.c_str(), &upperH, 180, NULL);

  cv::createTrackbar("LowerS", lightName.c_str(), &lowerS, 256, NULL);
  cv::createTrackbar("UpperS", lightName.c_str(), &upperS, 256, NULL);

  cv::createTrackbar("LowerV", lightName.c_str(), &lowerV, 256, NULL);
  cv::createTrackbar("UpperV", lightName.c_str(), &upperV, 256, NULL); 
}

int main(){
  cv::VideoCapture capture; 
  if (!capture.open(0)) {
    printf("Capture failure\n");
    return -1;
  }
  capture.set(CV_CAP_PROP_FRAME_WIDTH, kFrameWidth);
  capture.set(CV_CAP_PROP_FRAME_HEIGHT, kFrameHeight);
  cv::Mat frame;
  
  SetwindowSettings();

  //iterate through each frames of the video
  while(true){
 
    if (!capture.read(frame)) break;
    
    cv::Mat imgHSV(frame.size(), CV_8U, 3); 
    cv::cvtColor(frame, imgHSV, CV_BGR2HSV); //Change the color format from BGR to HSV
    
    cv::Mat imgThresh = GetThresholdedImage(imgHSV);
    
    cv::imshow(lightName.c_str(), imgThresh);
    cv::imshow(videoName.c_str(), frame);
    
    //Clean up used images
    imgHSV.release();
    imgThresh.release();
    frame.release();
    
    int c = cv::waitKey(waitDuration);
    //If 'ESC' is pressed, break the loop
    if((char)c==27 ) break;
    else if ((char)c=='v') {
      std::cout << "lowerH = " << lowerH << std::endl;
      std::cout << "upperH = " << upperH << std::endl;
      std::cout << "lowerS = " << lowerS << std::endl;
      std::cout << "upperS = " << upperS << std::endl;
      std::cout << "lowerV = " << lowerV << std::endl;
      std::cout << "upperV = " << upperV << std::endl;
    }
    
  }
  
  cv::destroyAllWindows();
  capture.release();
  
  return 0;
}
