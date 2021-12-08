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


#include "FileController.h"
#include "Server/Devices/DeviceManager.h"
#include "Server/Authentication/AuthentificationService.h"
#include "Server/Authentication/IUser.h"
#include <QUuid>
#include <QBuffer>


QMap<QString, FileController::TempFile> FileController::_files = QMap<QString, FileController::TempFile>();
QMutex FileController::_mutex;

FileController::FileController(QObject *parent) : IResourceHttpController(false)
{

}

void FileController::handleResourceOperation(QString token, IResourceHttpController::PathElements &pathElements, QString command, QVariantMap parameters, HttpRequest &request, HttpResponse &response)
{
    Q_UNUSED(command)
    Q_UNUSED(parameters)
    QString id = pathElements.id;

    _mutex.lock();
    if(_files.contains(id))
    {
        response.setHeader("Content-Type", "application/octet-stream");
        QBuffer stream;
        stream.open(QBuffer::ReadWrite);
        TempFile file = _files.take(id);
        _mutex.unlock();
        QTemporaryFile* tmpFile = file.file;
        if(tmpFile->open())
        {
            QDataStream stream(file.file);
            tmpFile->seek(0);
            while (!tmpFile->atEnd())
            {
                QByteArray buffer=tmpFile->read(65536);
                response.write(buffer);
            }
        }

        delete tmpFile;
        response.write("OK",true);
        return;
    }
    _mutex.unlock();

    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
    if(user.isNull())
        invalidToken(response);

    if (request.getParameter("upload") =="finish")
    {
        QTemporaryFile* file=request.getUploadedFile("file1");

        QString filename = request.getParameter("file1");
        QString path = pathElements.path;
        QStringList tokens = path.split("/");
        if (tokens.count() < 2)
        {
            invalidData(response, "Unknown endpoint");
            return;
        }

        QString receiverType = tokens.takeFirst();
        TempFile fileItem;
        if (file)
        {
            if(receiverType == "device")
            {
                QString deviceID = tokens.at(0 );
                deviceID = deviceID.replace(".", "/");
                if(!DeviceManager::instance()->getMappings().contains(deviceID))
                {
                    invalidData(response, "Device not found");
                    return;
                }

                deviceHandlePtr device = DeviceManager::instance()->getHandleByMapping(deviceID);

                fileItem.file = file;
                fileItem.fileName = filename;
                fileItem.id = QUuid::createUuid().toString().remove("{").remove("}");
                fileItem.sender = user->identityID();
                _mutex.lock();
                _files.insert(fileItem.id, fileItem);
                _mutex.unlock();
                file->setProperty("inUse", true);
                QVariantMap args = fileItem.toVariant();
                qDebug()<<fileItem.toVariant();
                IDevice::DeviceError err = device->triggerFunction("loadFile", fileItem.toVariant());
                if(err == IDevice::NO_ERROR)
                {
                    response.write("OK", true);
                    return;
                }
            }
        }

        response.setStatus(500, "Something went wrong...");
        response.write("Oohps, something went wrong..", true);
    }
}
