#include <opencv2/opencv.hpp>

namespace whiteboard {

std::pair<cv::Size, cv::Mat> transformation(const cv::Mat &image, size_t maxPixels);

}
