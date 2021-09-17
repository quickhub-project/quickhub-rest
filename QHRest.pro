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


QT += core websockets qml

DEFINES += HTTPSERVER_LIBRARY

include(../buildconfig.pri)
include(src/QtWebApp/httpserver/httpserver.pri)

SOURCES += src/HttpServer.cpp \
    src/Controller/FileController.cpp \
    src/RequestMapper.cpp \
    src/Controller/ListController.cpp \
    src/Controller/ObjectController.cpp \
    src/Controller/IResourceHttpController.cpp \
    src/HttpServerPlugin.cpp \
    src/Controller/ImageController.cpp \
    src/Controller/LoginController.cpp

HEADERS += src/HttpServer.h \
    src/Controller/FileController.h \
    src/RequestMapper.h \
    src/Controller/ListController.h \
    src/Controller/ObjectController.h \
    src/Controller/IResourceHttpController.h \
    src/HttpServerPlugin.h \
    src/HttpServer_global.h \
    src/Controller/ImageController.h \
    src/Controller/LoginController.h



#INSTALLS += target
#target.path = /home/pi/Server2/plugins
