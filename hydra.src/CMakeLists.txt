
#
# Projects the want to build on hydra should start their CMakeLists.txt with the following:
#
#CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
#
#FIND_PATH(HYDRA_SRC_PATH CMakeLists.txt DOC "Directory containing the hydra CMakeLists.txt and source code")
#IF(NOT HYDRA_SRC_PATH)
#  MESSAGE(FATAL_ERROR "HYDRA_SRC_PATH must be set")
#ENDIF(NOT HYDRA_SRC_PATH)
#INCLUDE(${HYDRA_SRC_PATH}/CMakeLists.txt)



CMAKE_POLICY(PUSH)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(hydra)

#we usually want shared so that qt is dynamic
SET(BUILD_SHARED_LIBS "YES" CACHE BOOL "Build shared libraries rather than static ones")

# this is useful is this file is ever INCLUDED by other files
GET_FILENAME_COMPONENT(THIS_PATH ${CMAKE_CURRENT_LIST_FILE} PATH)

IF(BUILD_SHARED_LIBS)
ADD_DEFINITIONS(-DBUILD_SHARED_LIBS)
ENDIF(BUILD_SHARED_LIBS)

#
# Compiler Adjustments
#

IF(UNIX)
ADD_DEFINITIONS(-Wall -Wno-sign-compare)
ENDIF(UNIX)

IF(WIN32)
ADD_DEFINITIONS(/w44018)    # sign compare warning
ADD_DEFINITIONS(/w44018)    # sign compare warning
ADD_DEFINITIONS(/D_CRT_SECURE_NO_WARNINGS)    # disable complaing about ::getenv
ENDIF(WIN32)

#
# QT
#

FIND_PACKAGE(Qt4)
SET(QT_USE_QTXML 1)
SET(QT_USE_QTMAIN 1)   # dont forget to add WIN32 to ADD_EXECUTABLE

INCLUDE(${QT_USE_FILE})
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}) #the UI generator puts ui_*.h files here

#
# sqlite3
#

FIND_LIBRARY(SQLITE3_LIBRARY sqlite3)

SET(SQLITE3_SRC_DIR "" CACHE PATH "The directory of sqlite3.c and sqlite3.h, an alternative to SQLITE3_LIBRARY")

#
# exif (optional)
#

SET(BUILD_HYDRA_LIBEXIF_SUPPORT OFF CACHE BOOL "Enable libexif usage in hydra")
IF(BUILD_HYDRA_LIBEXIF_SUPPORT)
FIND_LIBRARY(EXIF_LIBRARY exif)
ADD_DEFINITIONS(-DBUILD_HYDRA_EXIF_SUPPORT)
ENDIF(BUILD_HYDRA_LIBEXIF_SUPPORT)

QT4_WRAP_CPP(HEADERS_MOC ${THIS_PATH}/hydra/DirMonitor.h)
SET(SOURCES
    ${THIS_PATH}/hydra/ArgumentParser.cpp ${THIS_PATH}/hydra/NodePath.cpp ${THIS_PATH}/hydra/XMLUtil.cpp
    ${THIS_PATH}/hydra/Hash.cpp ${THIS_PATH}/hydra/FileIterator.cpp ${THIS_PATH}/hydra/FileOp.cpp ${THIS_PATH}/hydra/DirMonitor.cpp
    ${THIS_PATH}/hydra/Record.cpp ${THIS_PATH}/hydra/DB.cpp ${THIS_PATH}/hydra/Records.cpp ${THIS_PATH}/hydra/RotateCode.cpp
    ${THIS_PATH}/hydra/Query.cpp ${THIS_PATH}/hydra/Engine.cpp
    ${THIS_PATH}/hydra/Thumb.cpp ${THIS_PATH}/hydra/Exif.cpp
   )

IF(SQLITE3_SRC_DIR)
INCLUDE_DIRECTORIES(${SQLITE3_SRC_DIR})
SET(SOURCES ${SOURCES} ${SQLITE3_SRC_DIR}/sqlite3.c)
ENDIF(SQLITE3_SRC_DIR)

IF(WIN32 AND BUILD_SHARED_LIBS)
# under windows, libhydra is always static
# its just simpler that way (no need to export symbols, etc)
SET(ALWAYS_WIN32_STATIC STATIC)
ENDIF(WIN32 AND BUILD_SHARED_LIBS)

QT4_WRAP_UI(DESKTOP_UI ${THIS_PATH}/desktop/TagEditorView.ui)
QT4_WRAP_CPP(DESKTOP_MOC ${THIS_PATH}/desktop/FileList.h ${THIS_PATH}/desktop/MainWindow.h ${THIS_PATH}/desktop/VCRView.h
    ${THIS_PATH}/desktop/MainImageView.h ${THIS_PATH}/desktop/InfoView.h
    ${THIS_PATH}/desktop/WideListView.h ${THIS_PATH}/desktop/ListView.h ${THIS_PATH}/desktop/ThumbsView.h
    ${THIS_PATH}/desktop/QueryView.h ${THIS_PATH}/desktop/SearchView.h ${THIS_PATH}/desktop/ShuffleView.h ${THIS_PATH}/desktop/TagSelectView.h
    ${THIS_PATH}/desktop/TagEditorView.h ${THIS_PATH}/desktop/DirView.h)

INCLUDE_DIRECTORIES(${THIS_PATH})

ADD_LIBRARY(hydra ${ALWAYS_WIN32_STATIC} ${HEADERS_MOC} ${SOURCES})
TARGET_LINK_LIBRARIES(hydra ${QT_LIBRARIES})
IF(NOT SQLITE3_LIBRARY STREQUAL "SQLITE3_LIBRARY-NOTFOUND")
TARGET_LINK_LIBRARIES(hydra ${SQLITE3_LIBRARY})
ENDIF(NOT SQLITE3_LIBRARY STREQUAL "SQLITE3_LIBRARY-NOTFOUND")
IF(BUILD_HYDRA_LIBEXIF_SUPPORT)
TARGET_LINK_LIBRARIES(hydra ${EXIF_LIBRARY})
ENDIF(BUILD_HYDRA_LIBEXIF_SUPPORT)

ADD_EXECUTABLE(hydratag ${THIS_PATH}/hydra/hydratag.cpp ${THIS_PATH}/hydra/CommandImport.cpp)
TARGET_LINK_LIBRARIES(hydratag hydra)

ADD_EXECUTABLE(hydraweb ${THIS_PATH}/hydra/hydraweb.cpp ${THIS_PATH}/hydra/WebExport.cpp)
TARGET_LINK_LIBRARIES(hydraweb hydra)

ADD_EXECUTABLE(hydradesktop WIN32
    ${DESKTOP_MOC} ${DESKTOP_UI}
    ${THIS_PATH}/desktop/hydradesktop.cpp ${THIS_PATH}/desktop/MainWindow.cpp
    ${THIS_PATH}/desktop/FileList.cpp ${THIS_PATH}/desktop/FileEntry.cpp ${THIS_PATH}/desktop/FileEntryCache.cpp ${THIS_PATH}/desktop/FileItemCache.cpp
    ${THIS_PATH}/desktop/LoadCache.cpp ${THIS_PATH}/desktop/ImageCache.cpp ${THIS_PATH}/desktop/ThumbCache.cpp ${THIS_PATH}/desktop/FutureLoader.cpp
    ${THIS_PATH}/desktop/View.cpp
    ${THIS_PATH}/desktop/MainImageView.cpp ${THIS_PATH}/desktop/VCRView.cpp ${THIS_PATH}/desktop/InfoView.cpp
    ${THIS_PATH}/desktop/WideListView.cpp ${THIS_PATH}/desktop/ListView.cpp ${THIS_PATH}/desktop/ThumbsView.cpp
    ${THIS_PATH}/desktop/QueryView.cpp ${THIS_PATH}/desktop/SearchView.cpp ${THIS_PATH}/desktop/ShuffleView.cpp ${THIS_PATH}/desktop/TagSelectView.cpp
    ${THIS_PATH}/desktop/TagEditorView.cpp ${THIS_PATH}/desktop/DirView.cpp
    ${THIS_PATH}/desktop/SelectionPlugins.cpp ${THIS_PATH}/desktop/ViewPlugins.cpp
    ${THIS_PATH}/desktop/FlowLayout.cpp
    ${THIS_PATH}/desktop/RunnableEvent.cpp ${THIS_PATH}/desktop/FileOp.cpp
    )
TARGET_LINK_LIBRARIES(hydradesktop hydra)

# experiement wexus2 stuff
ADD_EXECUTABLE(wexusserver
    ${THIS_PATH}/wexus/wexusserver.cpp
    ${THIS_PATH}/wexus/HTTP.cpp ${THIS_PATH}/wexus/HTTPServer.cpp
    ${THIS_PATH}/wexus/HTTPHandlerStack.cpp
    ${THIS_PATH}/wexus/FileHTTPHandler.cpp ${THIS_PATH}/wexus/MimeTypes.cpp
    ${THIS_PATH}/wexus/Site.cpp ${THIS_PATH}/wexus/Application.cpp ${THIS_PATH}/wexus/Controller.cpp
    ${THIS_PATH}/wexus/MongooseServer.cpp ${THIS_PATH}/wexus/mongoose.c
    ${THIS_PATH}/webapps/PingerApp.cpp
    )
TARGET_LINK_LIBRARIES(wexusserver hydra)

CMAKE_POLICY(POP)

