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



#include "ListController.h"
#include "Server/Resources/ResourceManager/ResourceManager.h"
#include "Server/Resources/ListResource/ListResource.h"
#include "Server/Authentication/AuthentificationService.h"
#include "Server/Authentication/User.h"
#include "QJsonDocument"


ListController::ListController(): IResourceHttpController()
{
}

void ListController::handleResourceOperation(QString token, PathElements& pathElements, QString command, QVariantMap parameters, HttpRequest &request, HttpResponse &response)
{
    if(!pathElements.valid)
    {
        invalidData(response, "Path incomplete.");
        return;
    }

    QSharedPointer<ListResource> resource = getResource(pathElements.resource,"synclist", token, response).objectCast<ListResource>();
    if(!resource)
        return;

    int additionalElementCount = pathElements.additionalElements.count();

    if(command.toLower() == "insert" || (command.isEmpty() && (request.getMethod() == "POST") && additionalElementCount == 1))
    {
        QVariant data = parameters["data"];
        QVariant indexVariant = parameters["index"];

        if(!indexVariant.isValid() && additionalElementCount >= 1)
        {
            indexVariant = pathElements.additionalElements[0];
        }

        bool ok;
        int index = indexVariant.toInt(&ok);

        if(!data.isValid() || !indexVariant.isValid() || !ok)
        {
            invalidData(response, "No valid index.");
            return;
        }

        ListResource::ModificationResult result = resource->insertAt(data, index, token);
        handleMofidicationResult(result, response);
        return;
    }

    if(command.toLower() == "append" || (command.isEmpty() && (request.getMethod() == "POST")))
    {
        QVariant data = parameters["data"];
        if(!data.isValid())
        {
            invalidData(response);
            return;
        }
        ListResource::ModificationResult result = resource->appendItem(data, token);
        handleMofidicationResult(result, response);
        return;
    }

    if(command.toLower() == "get" || (command.isEmpty() && (request.getMethod() == "GET")))
    {
        QVariant var = resource->getListData();
        writeVariant(var, response);
        return;
    }

    if(command.toLower() == "remove" || (command.isEmpty() && (request.getMethod() == "DELETE") && additionalElementCount == 1))
    {
        QVariant uuidVariant = parameters["uuid"];
        if(!uuidVariant.isValid())
        {
            uuidVariant = pathElements.additionalElements[0];
        }

        QString uuid = uuidVariant.toString();
        if(uuid.isEmpty())
            invalidData(response, "No valid item uuid.");

        int index = -1;
        if(parameters.contains("index"))
        {
            bool ok;
            int tmpIndex = parameters["index"].toInt(&ok);
            if(ok)
                index = tmpIndex;
        }
        ListResource::ModificationResult result = resource->removeItem(uuid, token, index);
        handleError(result.error, response);
    }

    if(command.toLower() == "setproperty" || (command.isEmpty() && (request.getMethod() == "PATCH") && additionalElementCount == 1))
    {
        QVariant uuidVariant = parameters["uuid"];
        if(!uuidVariant.isValid())
        {
            uuidVariant = pathElements.additionalElements[0];
        }

        QString uuid = uuidVariant.toString();
        if(uuid.isEmpty())
            invalidData(response, "No valid item uuid.");

        int index = -1;
        if(parameters.contains("index"))
        {
            bool ok;
            int tmpIndex = parameters["index"].toInt(&ok);
            if(ok)
                index = tmpIndex;
        }

        if(request.getMethod() == "PATCH")
        {
            QVariant data = parameters["data"].toMap();
            if(data.isValid())
            {
                QVariantMap dataMap = data.toMap();
                QMapIterator<QString, QVariant> it(dataMap);
                while(it.hasNext())
                {
                    it.next();
                    ListResource::ModificationResult result = resource->setProperty(it.key(), it.value(), index, uuid, token);
                    handleMofidicationResult(result, response, !it.hasNext());
                }
                return;
            }
        }
        else
        {
            QString property = parameters["property"].toString();
            QVariant data = parameters["property"];
            if(property.isEmpty())
                invalidData(response, "Invalid property.");
            if(!data.isValid())
                invalidData(response, "Invalid data.");

            ListResource::ModificationResult result = resource->setProperty(property, data, index, uuid, token);
            handleMofidicationResult(result, response);
            return;
        }
    }

    if(command.toLower() == "delete" || (command.isEmpty() && (request.getMethod() == "DELETE")))
    {
        ListResource::ModificationResult result = resource->deleteList(token);
        handleError(result.error, response);
        return;
    }
}
