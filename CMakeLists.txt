cmake_minimum_required(VERSION 3.12)

project(PixelSorter)

add_subdirectory(ImageProcessingLibrary)

if (NOT TARGET MatrixModule)
	add_subdirectory(MatrixModule)
endif()

add_executable(PixelSorter Main.cpp)

target_include_directories(PixelSorter PUBLIC MatrixModule ImageProcessingLibrary)

target_link_libraries(PixelSorter PUBLIC ImageProcessingLibrary)