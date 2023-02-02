#include <opencv2/opencv.hpp>

namespace whiteboard {

std::pair<cv::Size, cv::Mat> transformation(const cv::Mat &image,
                                            size_t maxPixels);

namespace exceptions {
struct MissingCode {};
struct MissingUpperLeft : MissingCode {};
struct MissingBottomRight : MissingCode {};
struct BadPlacement {
  std::string msg;
};
} // namespace exceptions

} // namespace whiteboard
