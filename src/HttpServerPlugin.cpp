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


#include "HttpServerPlugin.h"
#include "HttpServer.h"
#include <QStandardPaths>

HttpServerPlugin::HttpServerPlugin(QObject *parent) : IPlugin(parent)
{

}

bool HttpServerPlugin::init(QVariantMap parameters)
{
    QString path =  parameters.value("f", QStandardPaths::standardLocations(QStandardPaths::DataLocation).at(0)+"/v1.3/").toString();
    new HttpServer(path);
    return true;
}

bool HttpServerPlugin::shutdown()
{
    return false;
}

QString HttpServerPlugin::getPluginName()
{
    return "QHRest";
}
