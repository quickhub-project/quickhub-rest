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


#include "ObjectController.h"
#include <QJsonDocument>

ObjectController::ObjectController() : IResourceHttpController()
{

}

void ObjectController::handleResourceOperation(QString token, PathElements& pathElements, QString command, QVariantMap parameters, HttpRequest &request, HttpResponse &response)
{
//    if(!pathElements.valid)
//    {
//        invalidData(response, "Path incomplete.");
//        return;
//    }

    qDebug()<<pathElements.resource;

    QSharedPointer<ObjectResource> resource = getResource(pathElements.resource, "object", token, response).objectCast<ObjectResource>();

    if(resource.isNull())
    {
        invalidData(response, "Resource not available");
        return;
    }

    if(command.toLower() == "get" || (command.isEmpty() && (request.getMethod() == "GET")))
    {

        QVariant var = resource->getObjectData();
        writeVariant(var, response);
        return;
    }

    if(command.toLower() == "set" || (command.isEmpty() && (request.getMethod() == "POST")))
    {
        QString property;
        QVariant propertyVariant = parameters["property"];
        QVariant data = parameters["data"];

        if(!propertyVariant.isValid())
        {

           // property = last;
        }
        else
        {
            property = propertyVariant.toString();
        }

        if(!data.isValid() | property.isEmpty())
        {
            qDebug()<< Q_FUNC_INFO - "No valid data. Nothing to append.";
            return;
        }

        ObjectResource::ModificationResult result = resource->setProperty(property, data, token);
        if(result.error == ObjectResource::NO_ERROR)
        {
            iIdentityPtr  user = AuthenticationService::instance()->validateToken(token);
            qDebug()<<user->identityID()+" appended data via http.";
        }
    }
}


