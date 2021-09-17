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



#include "IResourceHttpController.h"
#include <QJsonDocument>

#include "Server/Resources/ResourceManager/ResourceManager.h"
#include "Server/Authentication/AuthentificationService.h"
#include "Server/Authentication/User.h"
#include "httpsessionstore.h"

extern HttpSessionStore* sessionStore;

IResourceHttpController::IResourceHttpController(bool checkToken):
    _checkHttpTokens(checkToken)
{
}

/*
    Prepares the parameters and checks the context (session token, user parameters) for
    the derived Resource Controller.
*/
void IResourceHttpController::service(HttpRequest &request, HttpResponse &response)
{

    HttpSession session=sessionStore->getSession(request,response);
    QMultiMap<QByteArray,QByteArray> params = request.getParameterMap();
    QString token;
    QString path = QString::fromLatin1(request.getPath());


    if (session.contains("token"))
        token = session.get("token").toString();
    else
        token = QString::fromLatin1( params.value("token"));


    if(token.isEmpty() && _checkHttpTokens)
    {
        QString userName = request.getParameter("user");
        QString password = request.getParameter("pass");
        if(!userName.isEmpty() && !password.isEmpty())
        {
            AuthenticationService::ErrorCode errCode;
            token = AuthenticationService::instance()->login(userName, password, &errCode);
            if(errCode == AuthenticationService::NoError)
            {
                session.set("token", token);
            }
        }
        else
        {
           session.set("initial",path);
           response.redirect("/login");
           return;
        }
    }


    QString command = QString::fromLatin1( params.value("command"));

    if(path.right(1) == "/")
        path = path.remove(path.count()-1, 1);

    params.remove("token");
    params.remove("command");

    if(_checkHttpTokens)
    {
        qDebug()<< token<<": "<<command;

        iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
        if(user.isNull())
        {
            qDebug()<<Q_FUNC_INFO<<" - Invalid token.";
            invalidToken(response);
            return;
        }
    }

    QVariantMap parameters;
    QVariant body;
    QJsonParseError error;
    QJsonDocument bodyJson = QJsonDocument::fromJson(request.getBody(), &error);
    if(!bodyJson.isEmpty())
    {
        if(error.error != QJsonParseError::NoError)
            qDebug()<<Q_FUNC_INFO<<"- JSON Error: "<<error.errorString();
        else
            body = bodyJson.toVariant();
    }

    if(body.isValid())
    {
        parameters = body.toMap();
    }
    else
    {
        QMapIterator<QByteArray,QByteArray> it(params);
        while(it.hasNext())
        {
            it.next();
            parameters.insert(it.key(), it.value());
        }
    }

    PathElements elements = splitPath(path);
    handleResourceOperation(token, elements, command, parameters, request, response);
}

IResourceHttpController::PathElements IResourceHttpController::splitPath(QString path)
{
    PathElements result;
    QStringList tokens = path.split("/", QString::SkipEmptyParts);

    if(tokens.count() < 2)
    {
        result.valid = false;
        return result;
    }

    result.type = tokens[0];
    result.resource = tokens[1].replace(".","/");
    if(tokens.count() > 2)
    {
        result.id = tokens[2];
        result.path = result.resource +"/"+ result.id;

        for(int i = 3; i < tokens.count(); i++)
        {
            result.additionalElements.append(tokens[i]);
        }
    }

    result.valid = true;
    return result;
}

void IResourceHttpController::invalidData(HttpResponse &response, QString description)
{
    response.setStatus(400, "Invalid data / Insufficiant arguments.");
    response.write("Invalid data / Insufficiant arguments. "+description.toLatin1(),true);
}

void IResourceHttpController::invalidToken(HttpResponse &response)
{
    response.setStatus(403, "Invalid token.");
    response.write("Invalid token. Please log in and try again.",true);
}

void IResourceHttpController::permissionDenied(HttpResponse &response, QString description)
{
    response.setStatus(403, "Permission Denied.");
    response.write("Permission Denied. " + description.toLatin1(), true);
}

bool IResourceHttpController::handleError(IResource::ResourceError& error, HttpResponse& response, bool last)
{
    switch(error)
    {
        case IResource::NO_ERROR:
            response.setStatus(200, "OK");
            return false;
        case IResource::INVALID_PARAMETERS:
            invalidData(response);
            return true;
        case IResource::PERMISSION_DENIED:
            permissionDenied(response);
            return true;
        case IResource::UNKNOWN_ITEM:
            response.setStatus(404, "Unknown Item.");
            response.write("Unknown Item.",last);
            return true;
        default:
            response.setStatus(500, "Something went wrong...");
            response.write("Oohps, something went wrong..", last);
            return true;
    }
}

void IResourceHttpController::handleMofidicationResult(IResource::ModificationResult &result, HttpResponse &response, bool last)
{
    if(!handleError(result.error, response, last))
    {
        writeVariant(result.data, response, last);
    }
}

void IResourceHttpController::writeVariant(QVariant &variant, HttpResponse &response, bool isLast)
{
    response.write(QJsonDocument::fromVariant(variant).toJson(), isLast);
}

resourcePtr IResourceHttpController::getResource(QString path, QString type, QString token, HttpResponse &response)
{
    Err::CloudError error;
    resourcePtr resource = ResourceManager::instance()->getOrCreateResource(type, path, token, &error);

    if(error == Err::NO_ERROR)
        return resource;

    if(error == Err::INVALID_TOKEN)
        invalidToken(response);

    return resourcePtr();
}
