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


#include "ImageController.h"
#include "Server/Resources/ImageResource/ImageResource.h"
#include "Server/Resources/ResourceManager/ResourceManager.h"
#include <QBuffer>
#include <QMimeDatabase>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

ImageController::ImageController() : IResourceHttpController()
{
}

void ImageController::handleResourceOperation(QString token, PathElements &pathElements, QVariantMap parameters, HttpRequest &request, HttpResponse &response)
{
    Q_UNUSED(parameters)

    if (!pathElements.valid) {
        invalidData(response, "Path incomplete");
        return;
    }

    QByteArray method = request.getMethod();
    bool hasId = !pathElements.id.isEmpty();

    // POST /images/{resource} - upload image (multipart, key=file)
    if (method == "POST") {
        QTemporaryFile* file = request.getUploadedFile("file");
        QString filename = request.getParameter("file");
        if (!file) {
            invalidData(response, "No file uploaded (use multipart key 'file')");
            return;
        }

        Err::CloudError err;
        resourcePtr resource = ResourceManager::instance()->getOrCreateResource("imgcoll", pathElements.resource, token, &err);
        if (err != Err::NO_ERROR) {
            if (err == Err::INVALID_TOKEN)
                invalidToken(response);
            else
                sendJsonError(response, 500, "Could not access image resource");
            return;
        }

        QSharedPointer<ImageResource> imgResource = resource.objectCast<ImageResource>();
        if (!imgResource) {
            sendJsonError(response, 500, "Resource is not an image collection");
            return;
        }

        QImage image;
        if (!image.load(file, nullptr)) {
            invalidData(response, "Could not load image");
            return;
        }

        IResource::ModificationResult result = invokeOnOwnerThread(imgResource.data(), [&]() {
            auto returnVal =imgResource->insert(image, QVariant(), filename, token);
            imgResource.reset();
            return returnVal;
        });
        handleModificationResult(result, response);
        return;
    }

    // GET /images/{resource} - list all image IDs and metadata
    if (method == "GET" && !hasId) {
        Err::CloudError err;
        resourcePtr resource = ResourceManager::instance()->getOrCreateResource("imgcoll", pathElements.resource, token, &err);
        if (err != Err::NO_ERROR) {
            if (err == Err::INVALID_TOKEN)
                invalidToken(response);
            else
                sendJsonError(response, 500, "Could not access image resource");
            return;
        }

        QSharedPointer<ImageResource> imgResource = resource.objectCast<ImageResource>();
        if (!imgResource) {
            sendJsonError(response, 500, "Resource is not an image collection");
            return;
        }

        QStringList ids;
        QVariantMap metadata;
        invokeOnOwnerThread(imgResource.data(), [&]() {
            ids = imgResource->getAllImageIds(token);
            metadata = imgResource->getAllMetadata();
            imgResource.reset();
            return true;
        });

        QJsonObject result;
        result["ids"] = QJsonArray::fromStringList(ids);
        result["metadata"] = QJsonObject::fromVariantMap(metadata);
        response.setStatus(200, "OK");
        response.write(QJsonDocument(result).toJson(QJsonDocument::Compact), true);
        return;
    }

    // GET /images/{resource}/{id} - get image (binary response)
    if (method == "GET" && hasId) {
        Err::CloudError err;
        resourcePtr resource = ResourceManager::instance()->getOrCreateResource("imgcoll", pathElements.resource, token, &err);
        if (err != Err::NO_ERROR) {
            if (err == Err::INVALID_TOKEN)
                invalidToken(response);
            else
                sendJsonError(response, 500, "Could not access image resource");
            return;
        }

        QSharedPointer<ImageResource> imgResource = resource.objectCast<ImageResource>();
        if (!imgResource) {
            sendJsonError(response, 500, "Resource is not an image collection");
            return;
        }

        QImage img = invokeOnOwnerThread(imgResource.data(), [&]() {
            auto returnVal = imgResource->getImage(pathElements.id, token);
            imgResource.reset();
            return returnVal;
        });

        if (img.isNull()) {
            notFound(response, "Image not found");
            return;
        }

        response.setHeader("Content-Type", "image/png");
        QBuffer stream;
        stream.open(QBuffer::ReadWrite);
        img.save(&stream, "PNG");
        stream.seek(0);
        while (!stream.atEnd()) {
            QByteArray buffer = stream.read(65536);
            response.write(buffer);
        }
        return;
    }

    // DELETE /images/{resource}/{id} - delete image
    if (method == "DELETE" && hasId) {
        Err::CloudError err;
        resourcePtr resource = ResourceManager::instance()->getOrCreateResource("imgcoll", pathElements.resource, token, &err);
        if (err != Err::NO_ERROR) {
            if (err == Err::INVALID_TOKEN)
                invalidToken(response);
            else
                sendJsonError(response, 500, "Could not access image resource");
            return;
        }

        QSharedPointer<ImageResource> imgResource = resource.objectCast<ImageResource>();
        if (!imgResource) {
            sendJsonError(response, 500, "Resource is not an image collection");
            return;
        }

        IResource::ModificationResult result = invokeOnOwnerThread(imgResource.data(), [&]() {
            auto returnVal = imgResource->deleteImage(pathElements.id, token);
            imgResource.reset();
            return returnVal;
        });
        handleModificationResult(result, response);
        return;
    }

    sendJsonError(response, 405, "Method not allowed");
}
