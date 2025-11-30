cmake_minimum_required(VERSION 3.15..4.0)

project("CSC8503_Common" VERSION 1.0 LANGUAGES CXX)

add_library(CSC8503_Common)

target_sources(CSC8503_Common
  PUBLIC
  FILE_SET HEADERS
  PRIVATE
  serverCore.cpp
  ClientGame.cpp
  ServerGame.cpp
)

target_link_libraries(CSC8503_Common PUBLIC CSC8503)

add_executable(CSC8503_Client ClientMain.cpp)
add_executable(CSC8503_Server ServerMain.cpp)

target_link_libraries(CSC8503_Client PRIVATE CSC8503_Common)
target_link_libraries(CSC8503_Server PRIVATE CSC8503_Common)

include(setupProj)
SETUPPROJ(CSC8503_Common)
setupProj(CSC8503_Client)
setupProj(CSC8503_Server)
