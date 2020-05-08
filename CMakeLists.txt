cmake_minimum_required(VERSION 3.13.4)
project(algorithmic_languages_2_homework_2)

set(CMAKE_CXX_STANDARD 20)
include(${CMAKE_HOME_DIRECTORY}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(algorithmic_languages_2_homework_2 main.cpp)
target_link_libraries(algorithmic_languages_2_homework_2 ${CONAN_LIBS})