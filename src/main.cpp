#include <opencv2/aruco.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <iostream>

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cout << "usage: DisplayImage.out <Image_Path>\n";
    return -1;
  }

  cv::Mat image = cv::imread(argv[1], 1);
  if (!image.data) {
    std::cout << "No image data \n";
    return -1;
  }


  std::vector<int> markerIds;
  std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
  auto detectorParams = cv::aruco::DetectorParameters();
  auto dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
  cv::aruco::ArucoDetector detector(dictionary, detectorParams);
  detector.detectMarkers(image, markerCorners, markerIds, rejectedCandidates);

  auto outputImage = image.clone();
  cv::aruco::drawDetectedMarkers(outputImage, markerCorners, markerIds);

  cv::namedWindow("Display Image", cv::WINDOW_AUTOSIZE);
  cv::imshow("Display Image", outputImage);
  cv::waitKey(0);

  return 0;
}

