#include "whiteboard.hpp"

#include <optional>

namespace whiteboard {

using Points = std::vector<cv::Point2f>;

struct WhiteboardMarkers {
  Points topLeft;
  Points bottomRight;
};

static WhiteboardMarkers getBoardMarkers(const cv::Mat &image) {
  std::vector<int> markerIds;
  std::vector<Points> markerCorners;
  const auto detectorParams = cv::aruco::DetectorParameters();
  const auto dictionary =
      cv::aruco::getPredefinedDictionary(cv::aruco::DICT_ARUCO_ORIGINAL);
  cv::aruco::ArucoDetector detector(dictionary, detectorParams);
  detector.detectMarkers(image, markerCorners, markerIds);

  auto getCorner = [&](int findId) -> std::optional<Points> {
    for (size_t i{0}; i < markerIds.size(); ++i) {
      if (markerIds[i] == findId) {
        return markerCorners[i];
      }
    }
    return {};
  };

  auto ul = getCorner(11);
  if (!ul) {
    throw exceptions::MissingUpperLeft{};
  }
  auto br = getCorner(22);
  if (!br) {
    throw exceptions::MissingBottomRight{};
  }

  return WhiteboardMarkers{std::move(*ul), std::move(*br)};
}

static cv::Mat getInitialPerspectiveTransform(const Points &sourcePoints,
                                              const float factor = 1.0) {
  const Points destinationPoints = {
      {0.0, 0.0}, {factor, 0.0}, {factor, factor}, {0.0, factor}};

  return cv::findHomography(sourcePoints, destinationPoints);
}

static cv::Mat getSecondPerspectiveTransform(const WhiteboardMarkers &wbMarkers,
                                             const cv::Point2f &imageSize,
                                             const float arucoWidth) {
  Points sourcePoints;
  sourcePoints.reserve(8);
  std::copy(wbMarkers.topLeft.cbegin(), wbMarkers.topLeft.cend(),
            std::back_inserter(sourcePoints));
  std::copy(wbMarkers.bottomRight.cbegin(), wbMarkers.bottomRight.cend(),
            std::back_inserter(sourcePoints));

  const Points destinationPoints = {
      {0.0, 0.0},
      {arucoWidth, 0.0},
      {arucoWidth, arucoWidth},
      {0.0, arucoWidth},

      {imageSize.x - arucoWidth, imageSize.y - arucoWidth},
      {imageSize.x, imageSize.y - arucoWidth},
      {imageSize.x, imageSize.y},
      {imageSize.x - arucoWidth, imageSize.y}};

  return cv::findHomography(sourcePoints, destinationPoints);
}

static cv::Point2f transform(const cv::Mat &m, cv::Point2f p) {
  Points input{p};
  Points output{p};
  perspectiveTransform(input, output, m);
  return output[0];
};

static cv::Point2f
maxSizeWithProportion(int maxDimension,
                      const cv::Point2f &detectedProportions) {
  const auto maxDimF = float(maxDimension);

  if (detectedProportions.x <= 0.0) {
    throw exceptions::BadPlacement{
        "bottom-right code does not appear to be right of upper-left code"};
  }

  if (detectedProportions.y <= 0.0) {
    throw exceptions::BadPlacement{
        "bottom-right code does not appear to be lower than upper-left code"};
  }

  if (detectedProportions.y / detectedProportions.x > 1) {
    return {maxDimF * detectedProportions.x / detectedProportions.y, maxDimF};
  }
  return {maxDimF, maxDimF * detectedProportions.y / detectedProportions.x};
}

static cv::Mat shearingMatrix(const cv::Size &s, float n) {
  const float w = s.width, h = s.height;

  // clang-format off
  return cv::Mat_<double>(3, 3) <<
    n / 2.0 + 0.5,           (1.0 - n) / 2.0 * w / h, 0,
    (1.0 - n) / 2.0 * h / w, n / 2.0 + 0.5,           0,
    0,                       0,                       1;
  // clang-format on
}

static float boxShearFactor(const Points &ps, const cv::Mat M) {
  auto v = ps;
  for (auto &i : v) {
    i = transform(M, i);
  }
  return cv::norm(v[2] - v[0]) / cv::norm(v[3] - v[1]);
}

std::pair<cv::Size, cv::Mat>
transformation(const cv::Mat &image, size_t maxPixels, bool shearCorrection) {
  const auto boardMarkers = getBoardMarkers(image);
  const auto perspectiveMatrix =
      getInitialPerspectiveTransform(boardMarkers.topLeft);
  const auto br = transform(perspectiveMatrix, boardMarkers.bottomRight[2]);
  const auto imageSize = maxSizeWithProportion(1000, br);
  const auto arucoWidth = imageSize.x / br.x;
  auto undistortMatrix =
      getSecondPerspectiveTransform(boardMarkers, imageSize, arucoWidth);

  const auto imageCvSize = cv::Size(imageSize.x, imageSize.y);

  cv::Mat finalMatrix = undistortMatrix;

  if (shearCorrection) {
    const auto shearBr =
        boxShearFactor(boardMarkers.bottomRight, perspectiveMatrix);
    finalMatrix = shearingMatrix(imageCvSize, shearBr) * finalMatrix;
  }

  return {imageCvSize, std::move(finalMatrix)};
}

} // namespace whiteboard
