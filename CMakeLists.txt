cmake_minimum_required(VERSION 3.14)
project(snufflebrush)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall" )

add_subdirectory(libpng)

find_package(PNG REQUIRED)

include(FindPackageHandleStandardArgs)

set(png++_ROOT_DIR "" CACHE PATH "Folder contains png++")

find_path(png++_INCLUDE_DIR
        NAMES
        png++/color.hpp
        png++/config.hpp
        png++/consumer.hpp
        png++/convert_color_space.hpp
        png++/end_info.hpp
        png++/error.hpp
        png++/ga_pixel.hpp
        png++/generator.hpp
        png++/gray_pixel.hpp
        png++/image.hpp
        png++/image_info.hpp
        png++/index_pixel.hpp
        png++/info.hpp
        png++/info_base.hpp
        png++/io_base.hpp
        png++/packed_pixel.hpp
        png++/palette.hpp
        png++/pixel_buffer.hpp
        png++/pixel_traits.hpp
        png++/png.hpp
        png++/reader.hpp
        png++/require_color_space.hpp
        png++/rgb_pixel.hpp
        png++/rgba_pixel.hpp
        png++/streaming_base.hpp
        png++/tRNS.hpp
        png++/types.hpp
        png++/writer.hpp
        PATHS
        ${png++_ROOT_DIR}
        PATH_SUFFIXES
        src)

set(png++_INCLUDE_DIRS ${png++_INCLUDE_DIR} ${PNG_INCLUDE_DIRS})
set(png++_LIBRARIES ${PNG_LIBRARIES})

find_package_handle_standard_args(png++ DEFAULT_MSG
        png++_INCLUDE_DIR)

if(png++_FOUND)
    set(png++_INCLUDE_DIRS ${png++_INCLUDE_DIR})
    set(png++_LIBRARIES ${png++_LIBRARY})
endif()

include_directories(${png++_INCLUDE_DIR})
include_directories(${PNG_INCLUDE_DIR})
add_executable(snufflebrush main.cpp)
target_link_libraries(snufflebrush ${PNG_LIBRARY})