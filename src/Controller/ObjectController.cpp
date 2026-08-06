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
#include "Server/Resources/ObjectResource/ObjectResource.h"
#include "Server/Resources/ObjectResource/ObjectAccessProxy.h"
#include "Server/Authentication/AuthentificationService.h"

ObjectController::ObjectController() : IResourceHttpController()
{
}

void ObjectController::handleResourceOperation(QString token, PathElements& pathElements, QVariantMap parameters, HttpRequest &request, HttpResponse &response)
{
    if (!pathElements.valid) {
        invalidData(response, "Path incomplete");
        return;
    }

    QSharedPointer<ObjectResource> resource = getResource(pathElements.resource, "object", token, response).objectCast<ObjectResource>();
    if (resource.isNull()) {
        return;
    }

    ObjectAccessProxy proxy(resource);
    iIdentityPtr identity = AuthenticationService::instance()->validateToken(token);

    QByteArray method = request.getMethod();
    bool hasProperty = !pathElements.id.isEmpty();

    // GET /objects/{resource} - get full object
    // GET /objects/{resource}/{property} - get single property
    if (method == "GET") {
        if (hasProperty) {
            QString property = pathElements.id;
            QVariantMap objData = invokeOnOwnerThread(resource.data(), [&]() {
                auto data = proxy.getObjectData(identity);
                resource.reset();
                return data;
            });
            QVariant data = objData.value(property);
            if (!data.isValid()) {
                notFound(response, "Property not found");
                return;
            }
            writeVariant(data, response);
        } else {
            QVariantMap data = invokeOnOwnerThread(resource.data(), [&]() {
                auto data = proxy.getObjectData(identity);
                resource.reset();
                return data;
            });
            QVariant v(data);
            writeVariant(v, response);
        }
        return;
    }

    // PUT /objects/{resource}/{property} - set single property
    if (method == "PUT") {
        if (!hasProperty) {
            invalidData(response, "Missing property name");
            return;
        }
        QVariant data = parameters["data"];
        if (!data.isValid()) {
            invalidData(response, "Missing 'data' field");
            return;
        }
        QString property = pathElements.id;
        ObjectResource::ModificationResult result = invokeOnOwnerThread(resource.data(), [&]() {
            auto result = proxy.setProperty(property, data, token);
            resource.reset();
            return result;
        });
        handleModificationResult(result, response);
        return;
    }

    // PATCH /objects/{resource} - set multiple properties
    if (method == "PATCH") {
        QVariant dataVariant = parameters["data"];
        if (!dataVariant.isValid()) {
            invalidData(response, "Missing 'data' field");
            return;
        }
        QVariantMap dataMap = dataVariant.toMap();
        if (dataMap.isEmpty()) {
            invalidData(response, "Invalid data map");
            return;
        }
        ObjectResource::ModificationResult lastResult;
        invokeOnOwnerThread(resource.data(), [&]() {
            QMapIterator<QString, QVariant> it(dataMap);
            while (it.hasNext()) {
                it.next();
                lastResult = proxy.setProperty(it.key(), it.value(), token);
                if (lastResult.error != IResource::NO_ERROR)
                    break;
            }
            resource.reset();
            return true;
        });
        handleModificationResult(lastResult, response);
        return;
    }

    sendJsonError(response, 405, "Method not allowed");
}
