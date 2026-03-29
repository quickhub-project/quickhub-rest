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
#include "Server/Resources/ListResource/ListResource.h"

ListController::ListController() : IResourceHttpController()
{
}

void ListController::resolveId(const QString& id, int& outIndex, QString& outUuid)
{
    bool ok;
    int idx = id.toInt(&ok);
    if (ok) {
        outIndex = idx;
        outUuid = QString();
    } else {
        outIndex = -1;
        outUuid = id;
    }
}

void ListController::handleResourceOperation(QString token, PathElements& pathElements, QVariantMap parameters, HttpRequest &request, HttpResponse &response)
{
    if (!pathElements.valid) {
        invalidData(response, "Path incomplete");
        return;
    }

    QSharedPointer<ListResource> resource = getResource(pathElements.resource, "synclist", token, response).objectCast<ListResource>();
    if (!resource){
        return;
    }


    QByteArray method = request.getMethod();
    bool hasId = !pathElements.id.isEmpty();

    // GET /lists/{resource} - get full list
    // GET /lists/{resource}/{id} - get single item (id can be uuid or integer index)
    if (method == "GET") {
        if (hasId) {
            int index; QString uuid;
            resolveId(pathElements.id, index, uuid);
            QVariant item = invokeOnOwnerThread(resource.data(), [&]() {
                auto returnVal = resource->getItem(index, uuid);
                resource.reset();
                return returnVal;
            });
            if (!item.isValid()) {
                notFound(response, "Item not found");
                return;
            }
            writeVariant(item, response);
        } else {
            QVariantList data = invokeOnOwnerThread(resource.data(), [&]() {
                auto returnVal = resource->getListData();
                resource.reset();
                return returnVal;
            });
            QVariant v(data);
            writeVariant(v, response);
        }
        return;
    }

    // POST /lists/{resource} - append item
    // POST /lists/{resource}?index={n} - insert at index
    if (method == "POST") {
        QVariant data = parameters["data"];
        if (!data.isValid()) {
            invalidData(response, "Missing 'data' field");
            return;
        }

        QVariant indexVariant = parameters.value("index");
        if (indexVariant.isValid()) {
            bool ok;
            int index = indexVariant.toInt(&ok);
            if (!ok) {
                invalidData(response, "Invalid index");
                return;
            }
            ListResource::ModificationResult result = invokeOnOwnerThread(resource.data(), [&]() {
                auto result =resource->insertAt(data, index, token);
                resource.reset();
                return result;
            });
            handleModificationResult(result, response);
        } else {
            ListResource::ModificationResult result = invokeOnOwnerThread(resource.data(), [&]() {
                auto result = resource->appendItem(data, token);
                resource.reset();
                return result;
            });
            handleModificationResult(result, response);
        }
        return;
    }

    // PUT /lists/{resource}/{id} - replace item (id can be uuid or integer index)
    if (method == "PUT") {
        if (!hasId) {
            invalidData(response, "Missing item identifier");
            return;
        }
        QVariant data = parameters["data"];
        if (!data.isValid()) {
            invalidData(response, "Missing 'data' field");
            return;
        }
        int index; QString uuid;
        resolveId(pathElements.id, index, uuid);
        ListResource::ModificationResult result = invokeOnOwnerThread(resource.data(), [&]() {
            auto returnVal = resource->set(data, index, uuid, token);
            resource.reset();
            return returnVal;
        });
        handleModificationResult(result, response);
        return;
    }

    // PATCH /lists/{resource}/{id} - update properties (id can be uuid or integer index)
    if (method == "PATCH") {
        if (!hasId) {
            invalidData(response, "Missing item identifier");
            return;
        }
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
        int index; QString uuid;
        resolveId(pathElements.id, index, uuid);
        ListResource::ModificationResult lastResult;
        invokeOnOwnerThread(resource.data(), [&]() {
            QMapIterator<QString, QVariant> it(dataMap);
            while (it.hasNext()) {
                it.next();
                lastResult = resource->setProperty(it.key(), it.value(), index, uuid, token);
                if (lastResult.error != IResource::NO_ERROR)
                    break;
            }
            resource.reset();
            return true;
        });
        handleModificationResult(lastResult, response);
        return;
    }

    // DELETE /lists/{resource}/{id} - remove item (id can be uuid or integer index)
    // DELETE /lists/{resource} - delete entire list
    if (method == "DELETE") {
        if (hasId) {
            int index; QString uuid;
            resolveId(pathElements.id, index, uuid);
            ListResource::ModificationResult result = invokeOnOwnerThread(resource.data(), [&]() {
                auto result = resource->removeItem(uuid, token, index);
                resource.reset();
                return result;
            });
            handleModificationResult(result, response);
        } else {
            ListResource::ModificationResult result = invokeOnOwnerThread(resource.data(), [&]() {
                auto result = resource->deleteList(token);
                resource.reset();
                return result;
            });
            handleModificationResult(result, response);
        }
        return;
    }

    sendJsonError(response, 405, "Method not allowed");
}
