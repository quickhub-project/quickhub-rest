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
#include "Server/Authentication/AuthentificationService.h"
#include "Server/Authentication/IUser.h"
#include <QBuffer>

ImageController::ImageController() : IResourceHttpController()
{}


void ImageController::handleResourceOperation(QString token, IResourceHttpController::PathElements &pathElements, QString command, QVariantMap parameters, HttpRequest &request, HttpResponse &response)
{
    Q_UNUSED(parameters);
    qDebug()<<pathElements.resource;
    qDebug()<<pathElements.id;
    qDebug()<<pathElements.path;


    if (request.getParameter("upload") =="finish")
    {
        QTemporaryFile* file=request.getUploadedFile("file1");
        QString filename = request.getParameter("file1");
        if (file)
        {
            qDebug()<<pathElements.path;
            resourcePtr resource = ResourceManager::instance()->getOrCreateResource("imgcoll", pathElements.path, token);
            QSharedPointer<ImageResource> imgResource = resource.objectCast<ImageResource>();
            if(imgResource == nullptr)
            {
                invalidData(response, "Image not available");
                return;
            }
            QImage image;
            if(image.load(file,"JPG"))
            {
                qInfo()<<"JPEG received: "<< filename;
                IResource::ModificationResult result = imgResource->insert(image, QVariant(), filename, token);
                handleMofidicationResult(result, response);
            }
            else
                invalidData(response,"Could not load image");
        }
        else
        {
            response.setStatus(500, "Something went wrong...");
            response.write("Oohps, something went wrong..", true);
        }
        return;
    }


//    if(command == "upload")
//    {
//        response.setHeader("Content-Type", "text/html; charset=ISO-8859-1");
//        response.write("<html><body>");
//        response.write("Upload a JPEG image file<p>");
//        response.write("<form method=\"post\" enctype=\"multipart/form-data\">");
//        response.write("  <input type=\"hidden\" name=\"upload\" value=\"finish\">");
//        response.write("  File: <input type=\"file\" name=\"file1\"><br>");
//        response.write("  <input type=\"submit\">");
//        response.write("</form>");
//        response.write("</body></html>",true);
//    }

    resourcePtr resource = ResourceManager::instance()->getOrCreateResource("imgcoll", pathElements.resource.replace(".","/"), token);
    QSharedPointer<ImageResource> imgResource = resource.objectCast<ImageResource>();
    if(imgResource  != nullptr)
    {
        QImage img = imgResource->getImage(pathElements.id, token);
        if(!img.isNull())
        {
            response.setHeader("Content-Type", "image/jpeg");
            QBuffer stream;
            stream.open(QBuffer::ReadWrite);
            img.save(&stream,"JPG");
            stream.seek(0);
            while (!stream.atEnd())
            {
                QByteArray buffer=stream.read(65536);
                response.write(buffer);
            }
        }
    }
    else
    {

    }
}

