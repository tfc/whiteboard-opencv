#include <opencv2/aruco.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <optional>

using Points = std::vector<cv::Point2f>;

struct WhiteboardMarkers {
  Points topLeft;
  Points bottomRight;
};

std::optional<WhiteboardMarkers> getBoardMarkers(const cv::Mat &image) {
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
  auto br = getCorner(98);

  if (ul && br) {
    return WhiteboardMarkers{std::move(*ul), std::move(*br)};
  }
  return {};
}

cv::Mat getInitialPerspectiveTransform(const Points &sourcePoints, const float factor = 1.0) {
  const Points destinationPoints =
    { {0.0, 0.0}
    , {factor, 0.0}
    , {factor, factor}
    , {0.0, factor}
    };

  return cv::findHomography(sourcePoints, destinationPoints);
}

cv::Mat getSecondPerspectiveTransform(const WhiteboardMarkers &wbMarkers, const cv::Point2f &imageSize, const float arucoWidth) {
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

cv::Point2f transform(const cv::Mat &m, cv::Point2f p) {
  Points input {p};
  Points output {p};
  perspectiveTransform(input, output, m);
  return output[0];
};

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

  const auto oBoardMarkers = getBoardMarkers(image);

  if (!oBoardMarkers) {
    std::cout << "bad. no board markers.\n";
    return 1;
  }

  const auto perspectiveMatrix = getInitialPerspectiveTransform(oBoardMarkers->topLeft);

  const auto br = transform(perspectiveMatrix, oBoardMarkers->bottomRight[2]);

  const float widthPixels = 1000.0;
  cv::Point2f imageSize{widthPixels, widthPixels * br.y / br.x};
  const auto arucoWidth = imageSize.x / br.x;

  const auto finalMatrix = getSecondPerspectiveTransform(*oBoardMarkers, imageSize, arucoWidth);

  auto tImage = image.clone();
  cv::warpPerspective(image, tImage, finalMatrix, cv::Size(int(imageSize.x), int(imageSize.y)));

  cv::namedWindow("Display Image", cv::WINDOW_AUTOSIZE);
  cv::imshow("Display Image", tImage);
  cv::waitKey(0);

  return 0;
}

