# CMakeLists.txt --- CMake project settings
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 3.0)

# project name and languages
project(pdfium_test CXX)

# Use pdfium library (Please specify "PDFium_DIR" environment variable)
find_package(PDFium)

# libttc2ttf.a
add_subdirectory(ttc2ttf)

# read.exe
add_executable(read read.cpp SaveBitmapToFile.cpp)
target_include_directories(read PRIVATE "${PDFium_INCLUDE_DIR}")
target_link_libraries(read PRIVATE pdfium)

# write.exe
add_executable(write write.cpp)
target_include_directories(write PRIVATE "${PDFium_INCLUDE_DIR}")
target_link_libraries(write PRIVATE pdfium libttc2ttf)
