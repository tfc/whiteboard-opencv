#include "whiteboard.hpp"

#include <opencv2/calib3d.hpp>
#include <opencv2/opencv.hpp>

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

  const auto [wbImageSize, whiteboardMatrix] = whiteboard::transformation(image, 1000);

  auto tImage = image.clone();
  cv::warpPerspective(image, tImage, whiteboardMatrix, wbImageSize);

  cv::namedWindow("Display Image", cv::WINDOW_AUTOSIZE);
  cv::imshow("Display Image", tImage);
  cv::waitKey(0);

  return 0;
}

