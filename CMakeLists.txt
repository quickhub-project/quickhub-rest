# This file is part of the REST Plugin for the QuickHub framework
# (git@github.com:quickhub-project/quickhub-rest.git).
# Copyright (c) 2021 Friedemann Metzger
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.26)

project(quickhub-rest VERSION 0.1.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core WebSockets Qml Network)

qt_standard_project_setup()

qt_add_library(QHRest MODULE
    # Main HTTP server plugin sources
    src/Controller/FileController.cpp
    src/Controller/FileController.h
    src/Controller/ImageController.cpp
    src/Controller/ImageController.h
    src/Controller/IResourceHttpController.cpp
    src/Controller/IResourceHttpController.h
    src/Controller/ListController.cpp
    src/Controller/ListController.h
    src/Controller/LoginController.cpp
    src/Controller/LoginController.h
    src/Controller/ObjectController.cpp
    src/Controller/ObjectController.h
    src/HttpServer.cpp
    src/HttpServer.h
    src/HttpServer_global.h
    src/HttpServerPlugin.cpp
    src/HttpServerPlugin.h
    src/RequestMapper.cpp
    src/RequestMapper.h
    # QtWebApp HTTP server sources
    src/QtWebApp/httpserver/httpconnectionhandler.cpp
    src/QtWebApp/httpserver/httpconnectionhandler.h
    src/QtWebApp/httpserver/httpconnectionhandlerpool.cpp
    src/QtWebApp/httpserver/httpconnectionhandlerpool.h
    src/QtWebApp/httpserver/httpcookie.cpp
    src/QtWebApp/httpserver/httpcookie.h
    src/QtWebApp/httpserver/httpglobal.cpp
    src/QtWebApp/httpserver/httpglobal.h
    src/QtWebApp/httpserver/httplistener.cpp
    src/QtWebApp/httpserver/httplistener.h
    src/QtWebApp/httpserver/httprequest.cpp
    src/QtWebApp/httpserver/httprequest.h
    src/QtWebApp/httpserver/httprequesthandler.cpp
    src/QtWebApp/httpserver/httprequesthandler.h
    src/QtWebApp/httpserver/httpresponse.cpp
    src/QtWebApp/httpserver/httpresponse.h
    src/QtWebApp/httpserver/httpsession.cpp
    src/QtWebApp/httpserver/httpsession.h
    src/QtWebApp/httpserver/httpsessionstore.cpp
    src/QtWebApp/httpserver/httpsessionstore.h
    src/QtWebApp/httpserver/staticfilecontroller.cpp
    src/QtWebApp/httpserver/staticfilecontroller.h
)

target_compile_definitions(QHRest PRIVATE
    HTTPSERVER_LIBRARY
)

# Handle version-specific defines for Qt
if(Qt6_VERSION VERSION_GREATER_EQUAL 5.14.0)
    target_compile_definitions(QHRest PRIVATE
        SKIP_EMPTY_PARTS=Qt::SkipEmptyParts
    )
else()
    target_compile_definitions(QHRest PRIVATE
        SKIP_EMPTY_PARTS=QString::SkipEmptyParts
    )
endif()

target_link_libraries(QHRest PRIVATE
    Qt6::Core
    Qt6::WebSockets
    Qt6::Qml
    Qt6::Network
    QHCore
    2log-core
    QHPluginSystem
)

target_include_directories(QHRest PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Controller>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/QtWebApp/httpserver>
    $<INSTALL_INTERFACE:include/quickhub-rest>
)

# Set output directory to plugins
set_target_properties(QHRest PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugins
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugins
)

install(TARGETS QHRest
    EXPORT QHRestTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/plugins
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/plugins
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/plugins
)

install(DIRECTORY src/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/quickhub-rest
    FILES_MATCHING PATTERN "*.h"
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/QHRestConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/QHRestConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/quickhub-rest
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/QHRestConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/QHRestConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/QHRestConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/quickhub-rest
)

install(EXPORT QHRestTargets
    FILE QHRestTargets.cmake
    NAMESPACE quickhub::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/quickhub-rest
)
