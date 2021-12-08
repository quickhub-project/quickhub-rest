/* This file is part of the REST Plugin for the QuickHub framework
* (git@github.com:quickhub-project/quickhub-rest.git).
* Copyright (c) 2021 Friedemann Metzger - www.quickhub.org
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, version 3.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#include <QProcessEnvironment>
#include "HttpServer.h"
#include <QSettings>
#include "httplistener.h"
#include "RequestMapper.h"
#include "staticfilecontroller.h"
#include "httpsessionstore.h"
#include <QCoreApplication>
#include <QDir>

using namespace stefanfrings;
HttpSessionStore* sessionStore;
StaticFileController* staticFileController;
HttpServer::HttpServer(QString storagePath, QObject *parent) : QObject(parent)
{
    QSettings* listenerSettings;

    QString configFile = storagePath+"/httpServer/httpConfig.ini";
    if( false)
    {
        listenerSettings=new QSettings(configFile, QSettings::IniFormat);
        listenerSettings->beginGroup("listener");
    }
    else
    {
        qDebug()<<"Settings file not found. Use default Settings.";
        listenerSettings = new QSettings();
        listenerSettings->beginGroup("listener");
        listenerSettings->setValue("port", 8080);
        listenerSettings->setValue("maxMultiPartSize", 120000000);
    }

    QSettings fileSettings;

    QString docrootPath = QProcessEnvironment::systemEnvironment().value("HTTP_DOCROOT", "");
    if(docrootPath.isEmpty())
        QString docrootPath = storagePath+"/httpServer/docroot";

    qDebug()<<"HTTP docroot is: "+docrootPath;

    QDir dir(docrootPath);
    if(!dir.exists())
    {
        dir.mkpath(docrootPath);
    }


    QSettings* sessionSettings=new QSettings(configFile,QSettings::IniFormat, this);
    sessionSettings->beginGroup("sessions");
    sessionSettings->setValue("expirationTime", 60000*30);
    sessionStore=new HttpSessionStore(sessionSettings, this);

    fileSettings.setValue("path",docrootPath);
    staticFileController= new StaticFileController(&fileSettings, this);
    new HttpListener(listenerSettings, new RequestMapper(this),this);
}
