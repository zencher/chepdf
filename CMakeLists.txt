CMAKE_MINIMUM_REQUIRED(VERSION 3.6)
#project name
PROJECT(chepdf)
#head file path
INCLUDE_DIRECTORIES(include)
#source directory
AUX_SOURCE_DIRECTORY(source DIR_SRCS)
#add micro
ADD_DEFINITIONS(-std=c++11)
#add executable file
ADD_LIBRARY(chepdf ${DIR_SRCS})

find_package(zlib REQUIRED)
find_package(jpeg REQUIRED)
find_package(openjpeg REQUIRED)
find_package(freetype REQUIRED)

INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIRS} ${JPEG_INCLUDE_DIRS} ${OPENJPEG_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS} "/usr/local/include")

IF (APPLE)
    ADD_DEFINITIONS(-D_MAC_OS_X_)
ENDIF