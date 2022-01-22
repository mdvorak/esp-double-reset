cmake_minimum_required(VERSION 3.13.4)

# In-place use of library in this example
list(APPEND EXTRA_COMPONENT_DIRS " ${CMAKE_SOURCE_DIR}/..")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(double_reset_example)
