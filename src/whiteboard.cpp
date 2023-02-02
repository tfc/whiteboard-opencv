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
  std::vector<Points> markerCorners, rejectedCandidates;
  const auto detectorParams = cv::aruco::DetectorParameters();
  const auto dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
  cv::aruco::ArucoDetector detector(dictionary, detectorParams);
  detector.detectMarkers(image, markerCorners, markerIds, rejectedCandidates);

  auto getCorner = [&](int findId) -> std::optional<Points> {
    for (size_t i {0}; i < markerIds.size(); ++i) {
      if (markerIds[i] == findId) {
        return markerCorners[i];
      }
    }
    return {};
  };

  auto ul = getCorner(203);
  if (!ul) {
    throw exceptions::MissingUpperLeft{};
  }
  auto br = getCorner(98);
  if (!br) {
    throw exceptions::MissingBottomRight{};
  }

  return WhiteboardMarkers{std::move(*ul), std::move(*br)};
}

static cv::Mat getInitialPerspectiveTransform(const Points &sourcePoints, const float factor = 1.0) {
  const Points destinationPoints =
    { {0.0, 0.0}
    , {factor, 0.0}
    , {factor, factor}
    , {0.0, factor}
    };

  return cv::findHomography(sourcePoints, destinationPoints);
}

static cv::Mat getSecondPerspectiveTransform(const WhiteboardMarkers &wbMarkers, const cv::Point2f &imageSize, const float arucoWidth) {
  Points sourcePoints;
  sourcePoints.reserve(8);
  std::copy(wbMarkers.topLeft.cbegin(), wbMarkers.topLeft.cend(), std::back_inserter(sourcePoints));
  std::copy(wbMarkers.bottomRight.cbegin(), wbMarkers.bottomRight.cend(), std::back_inserter(sourcePoints));

  const Points destinationPoints =
    { {0.0, 0.0}
    , {arucoWidth, 0.0}
    , {arucoWidth, arucoWidth}
    , {0.0, arucoWidth}

    , {imageSize.x - arucoWidth, imageSize.y - arucoWidth}
    , {imageSize.x, imageSize.y - arucoWidth}
    , {imageSize.x, imageSize.y}
    , {imageSize.x - arucoWidth, imageSize.y}
    };

  return cv::findHomography(sourcePoints, destinationPoints);
}

static cv::Point2f transform(const cv::Mat &m, cv::Point2f p) {
  Points input {p};
  Points output {p};
  perspectiveTransform(input, output, m);
  return output[0];
};

static cv::Point2f maxSizeWithProportion(int maxDimension, const cv::Point2f &detectedProportions) {
  const auto maxDimF = float(maxDimension);

  if (detectedProportions.x <= 0.0) {
    throw exceptions::BadPlacement{"bottom-right code does not appear to be right of upper-left code"};
  }

  if (detectedProportions.y <= 0.0) {
    throw exceptions::BadPlacement{"bottom-right code does not appear to be lower than upper-left code"};
  }

  if (detectedProportions.y / detectedProportions.x > 1) {
    return {maxDimF * detectedProportions.x / detectedProportions.y, maxDimF};
  }
  return {maxDimF, maxDimF * detectedProportions.y / detectedProportions.x};
}

std::pair<cv::Size, cv::Mat> transformation(const cv::Mat &image, size_t maxPixels) {
  const auto boardMarkers = getBoardMarkers(image);
  const auto perspectiveMatrix = getInitialPerspectiveTransform(boardMarkers.topLeft);
  const auto br = transform(perspectiveMatrix, boardMarkers.bottomRight[2]);
  const auto imageSize = maxSizeWithProportion(1000, br);
  const auto arucoWidth = imageSize.x / br.x;
  auto finalMatrix = getSecondPerspectiveTransform(boardMarkers, imageSize, arucoWidth);

  return {cv::Size(imageSize.x, imageSize.y), std::move(finalMatrix)};
}

}