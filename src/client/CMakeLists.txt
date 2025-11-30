cmake_minimum_required(VERSION 3.15..4.0)

project("CSC8503_Client" VERSION 1.0 LANGUAGES CXX)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
PUBLIC
  FILE_SET HEADERS
PRIVATE
  ClientGame.cpp
  main.cpp
 "menus/solo.h" "menus/multiplayer.h" "menus/host.h" "menus/join.h" "menus/connectionGuard.h")

target_link_libraries(${PROJECT_NAME} PRIVATE CSC8503::CSC8503)

include(setupProj)
setupProj(${PROJECT_NAME})
