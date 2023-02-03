#include "whiteboard.hpp"

#include <opencv2/calib3d.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cout
        << "usage: DisplayImage.out <input_image_path> [output_image_path]\n";
    return -1;
  }

  cv::Mat image = cv::imread(argv[1], 1);
  if (!image.data) {
    std::cout << "No image data \n";
    return -1;
  }

  try {
    const auto [wbImageSize, whiteboardMatrix] =
        whiteboard::transformation(image, 1000);

    auto tImage = image.clone();
    cv::warpPerspective(image, tImage, whiteboardMatrix, wbImageSize);

    if (argc == 3) {
      cv::imwrite(argv[2], tImage);
    }

    cv::namedWindow("Display Image", cv::WINDOW_AUTOSIZE);
    cv::imshow("Display Image", tImage);
    cv::waitKey(0);
  } catch (const whiteboard::exceptions::MissingUpperLeft &) {
    std::cerr << "Error: Upper Left code (the one with ID 11) is missing\n";
    return 1;
  } catch (const whiteboard::exceptions::MissingBottomRight &) {
    std::cerr << "Error: Bottom Right code (the one with ID 22) is missing\n";
    return 1;
  } catch (const whiteboard::exceptions::BadPlacement &e) {
    std::cerr << "Error: The two ARUCO codes are misplaced (" << e.msg << ")\n";
    return 1;
  }

  return 0;
}
