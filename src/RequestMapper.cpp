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



#include "RequestMapper.h"
#include "Controller/ListController.h"
#include "Controller/ObjectController.h"
#include "Controller/ImageController.h"
#include "Controller/FileController.h"
#include "Controller/LoginController.h"
#include "Controller/ServiceController.h"
#include "staticfilecontroller.h"
#include "httpsessionstore.h"
#include "Server/Authentication/AuthentificationService.h"

extern StaticFileController* staticFileController;
extern HttpSessionStore* sessionStore;

RequestMapper::RequestMapper(QObject* parent) : HttpRequestHandler(parent)
{
    connect(sessionStore, &HttpSessionStore::sessionExpired, this, &RequestMapper::sessionExpired);
}

RequestMapper::~RequestMapper()
{
}

void RequestMapper::service(HttpRequest& request, HttpResponse& response)
{
    QString path = QString::fromLatin1(request.getPath());
    QStringList tokens = path.split("/", Qt::SkipEmptyParts);
    QString firstElement = path;
    if (tokens.count() > 0)
        firstElement = tokens.first();

    QString route = firstElement.toLower();

    if (route == "login" || route == "logout") {
        LoginController().service(request, response);
        return;
    }

    if (route == "lists") {
        ListController().service(request, response);
        return;
    }

    if (route == "objects") {
        ObjectController().service(request, response);
        return;
    }

    if (route == "images") {
        ImageController().service(request, response);
        return;
    }

    if (route == "services") {
        ServiceController().service(request, response);
        return;
    }

    if (route == "files") {
        FileController().service(request, response);
        return;
    }

    staticFileController->service(request, response);
}

void RequestMapper::sessionExpired(QString token)
{
    AuthenticationService::instance()->logout(token);
}
