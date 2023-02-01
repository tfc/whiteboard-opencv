cmake_minimum_required(VERSION 3.20)

project(WhiteboardOpencv)

find_package(OpenCV
  REQUIRED
  highgui
  objdetect
  )

add_executable(DisplayImage
  src/main.cpp
  )

target_link_libraries(DisplayImage ${OpenCV_LIBS})
