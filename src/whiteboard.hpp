#include <opencv2/opencv.hpp>

namespace whiteboard {

std::pair<cv::Size, cv::Mat> transformation(const cv::Mat &, size_t, bool);

namespace exceptions {
struct MissingCode {};
struct MissingUpperLeft : MissingCode {};
struct MissingBottomRight : MissingCode {};
struct BadPlacement {
  std::string msg;
};
} // namespace exceptions

} // namespace whiteboard
