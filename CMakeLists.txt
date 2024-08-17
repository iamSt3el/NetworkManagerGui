cmake_minimum_required(VERSION 3.10)
project(NetworkManagerGUI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)

include_directories(${GTKMM_INCLUDE_DIRS})
link_directories(${GTKMM_LIBRARY_DIRS})

add_executable(NetworkManagerGUI src/main.cpp)
target_link_libraries(NetworkManagerGUI ${GTKMM_LIBRARIES})

install(TARGETS NetworkManagerGUI RUNTIME DESTINATION bin)
install(FILES org.example.NetworkManagerGUI.desktop DESTINATION share/applications)
install(FILES org.example.NetworkManagerGUI.metainfo.xml DESTINATION share/metainfo)