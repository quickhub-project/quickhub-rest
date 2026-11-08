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
#include "Server/Authentication/AuthentificationService.h"
#include <QUuid>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QMutexLocker>

QMutex FileController::_mutex;

FileController::FileController() : IResourceHttpController(true)
{
}

QString FileController::uploadDir()
{
    QString dir = qEnvironmentVariable("FILE_UPLOAD_DIR");
    if (dir.isEmpty()) {
        dir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/uploads";
    }
    QDir().mkpath(dir);
    return dir;
}

void FileController::handleResourceOperation(QString token, PathElements &pathElements, QVariantMap parameters, HttpRequest &request, HttpResponse &response)
{
    Q_UNUSED(parameters)

    QByteArray method = request.getMethod();
    QString path = QString::fromLatin1(request.getPath());
    QStringList segments = path.split("/", Qt::SkipEmptyParts);

    // POST /files - upload file
    if (method == "POST" && segments.count() == 1) {
        QTemporaryFile* tmpFile = request.getUploadedFile("file");
        QString filename = request.getParameter("file");
        if (!tmpFile) {
            invalidData(response, "No file uploaded (use multipart key 'file')");
            return;
        }

        QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        QString dir = uploadDir();

        QMimeDatabase mimeDb;
        QString mimeType;
        if (!filename.isEmpty()) {
            mimeType = mimeDb.mimeTypeForFile(filename).name();
        }
        if (mimeType.isEmpty() || mimeType == "application/octet-stream") {
            if (tmpFile->open()) {
                tmpFile->seek(0);
                QByteArray header = tmpFile->read(1024);
                mimeType = mimeDb.mimeTypeForData(header).name();
            }
        }

        QString destPath = dir + "/" + id;
        if (tmpFile->open()) {
            tmpFile->seek(0);
            QFile destFile(destPath);
            if (!destFile.open(QIODevice::WriteOnly)) {
                sendJsonError(response, 500, "Could not write file to storage");
                return;
            }
            while (!tmpFile->atEnd()) {
                destFile.write(tmpFile->read(65536));
            }
            destFile.close();
        } else {
            sendJsonError(response, 500, "Could not read uploaded file");
            return;
        }

        // Write metadata sidecar
        QJsonObject meta;
        meta["id"] = id;
        meta["filename"] = filename;
        meta["mimeType"] = mimeType;
        meta["token"] = token;
        meta["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

        QFile metaFile(destPath + ".json");
        if (metaFile.open(QIODevice::WriteOnly)) {
            metaFile.write(QJsonDocument(meta).toJson(QJsonDocument::Compact));
            metaFile.close();
        }

        Q_EMIT fileUploaded(id, filename, token);

        QJsonObject result;
        result["id"] = id;
        result["filename"] = filename;
        result["mimeType"] = mimeType;
        result["url"] = "/files/" + id;
        response.setStatus(201, "Created");
        response.write(QJsonDocument(result).toJson(QJsonDocument::Compact), true);
        return;
    }

    // GET /files/{id} - download file
    if (method == "GET" && segments.count() >= 2) {
        QString id = segments[1];
        QString filePath = uploadDir() + "/" + id;
        QString metaPath = filePath + ".json";

        QFile file(filePath);
        if (!file.exists()) {
            notFound(response, "File not found");
            return;
        }

        QString mimeType = "application/octet-stream";
        QFile metaFile(metaPath);
        if (metaFile.open(QIODevice::ReadOnly)) {
            QJsonObject meta = QJsonDocument::fromJson(metaFile.readAll()).object();
            metaFile.close();
            if (meta.contains("mimeType"))
                mimeType = meta["mimeType"].toString();
        }

        response.setHeader("Content-Type", mimeType.toLatin1());
        if (!file.open(QIODevice::ReadOnly)) {
            sendJsonError(response, 500, "Could not read file");
            return;
        }
        while (!file.atEnd()) {
            response.write(file.read(65536));
        }
        file.close();
        return;
    }

    // DELETE /files/{id} - delete file
    if (method == "DELETE" && segments.count() >= 2) {
        QString id = segments[1];
        QString filePath = uploadDir() + "/" + id;
        QString metaPath = filePath + ".json";

        if (!QFile::exists(filePath)) {
            notFound(response, "File not found");
            return;
        }

        QMutexLocker locker(&_mutex);
        QFile::remove(filePath);
        QFile::remove(metaPath);
        locker.unlock();

        writeJsonSuccess(response);
        return;
    }

    sendJsonError(response, 405, "Method not allowed");
}
