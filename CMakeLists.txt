cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall)

project(WhiteboardOpencv)

find_package(OpenCV
  REQUIRED
  highgui
  objdetect
  )

add_executable(DisplayImage
  src/main.cpp
  )

set_property(TARGET DisplayImage
  PROPERTY CXX_STANDARD 20
  )


target_link_libraries(DisplayImage ${OpenCV_LIBS})
