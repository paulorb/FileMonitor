project(filemonitortest)

cmake_minimum_required(VERSION 2.8)

# For some external project macros
include(ExternalProject)

# Download boost from git and build regex module

ExternalProject_Add(
    boost
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/boost
    GIT_REPOSITORY https://github.com/ryppl/boost-svn.git
    GIT_TAG "Boost_1_41_0"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND bjam --with-thread --with-system toolset=gcc variant=debug link=static install --prefix=${CMAKE_CURRENT_BINARY_DIR}/boostinstall
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND ""
)

set(Boost_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/boostinstall/include)
set(Boost_LIBRARY_THREAD ${CMAKE_CURRENT_BINARY_DIR}/boostinstall/lib/libboost_thread.a)

# Configure app

include_directories(${Boost_INCLUDE_DIRS})
add_executable(filemonitortest TestFileMonitor.cpp)
add_dependencies(filemonitortest boost)
target_link_libraries(filemonitortest ${Boost_LIBRARY_THREAD})