cmake_minimum_required(VERSION 3.21)
project(coroutine-test LANGUAGES CXX VERSION 2.0.0)

set(CMAKE_CXX_STANDARD 20)

file(GLOB sources "*.cpp")
list(FILTER sources EXCLUDE REGEX "channel_race_condition.cpp|linux_event.*.cpp|windows.*")

foreach(src ${sources})
    get_filename_component(filename ${src} NAME)
    get_filename_component(name ${src} NAME_WLE)

    add_executable(${name} ${filename})

    target_include_directories(${name} PUBLIC
                                /opt/homebrew/Cellar/cpp-gsl/4.0.0_1/include
                                ../src/)

endforeach()

#add_executable(article_russian_roulette article_russian_roulette.cpp)

#target_include_directories(article_russian_roulette PUBLIC
#                            /opt/homebrew/Cellar/cpp-gsl/4.0.0_1/include
#                            ../src/)
