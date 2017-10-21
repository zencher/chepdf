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

CONFIGURE_FILE(
    "include/config.h.in"
    "include/config.h"
)

MESSAGE(STATUS "###################################")
 
MESSAGE(STATUS "operation system is ${CMAKE_SYSTEM}")
 
IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    MESSAGE(STATUS "current platform: Linux ")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    MESSAGE(STATUS "current platform: Windows")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    MESSAGE(STATUS "current platform: FreeBSD")
ELSE ()
    MESSAGE(STATUS "other platform: ${CMAKE_SYSTEM_NAME}")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")
 
IF (WIN32)
    MESSAGE(STATUS "Now is windows")
ELSEIF (APPLE)
    MESSAGE(STATUS "Now is Apple systens.")
    ADD_DEFINITIONS(-D_MAC_OS_X_)
ELSEIF (UNIX)
    MESSAGE(STATUS "Now is UNIX-like OS's. Including aPPLE os x  and CygWin")
ENDIF ()

MESSAGE(STATUS "###################################")