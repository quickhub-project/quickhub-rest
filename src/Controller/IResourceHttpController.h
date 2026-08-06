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


#ifndef IRESOURCEHTTPCONTROLLER_H
#define IRESOURCEHTTPCONTROLLER_H

#include <QObject>
#include <QThread>
#include <QMetaObject>
#include "httprequesthandler.h"
#include "Server/Resources/ResourceManager/IResource.h"

using namespace stefanfrings;
class User;
class IResourceHttpController : public HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(IResourceHttpController)

public:
    struct PathElements
    {
        bool valid = false;
        QString type;
        QString resource;
        QString id;
        QString path;
        QStringList additionalElements;
    };

    IResourceHttpController(bool checkToken = true);
    void service(HttpRequest& request, HttpResponse& response);
    virtual void handleResourceOperation(QString token, PathElements& pathElements, QVariantMap parameters, HttpRequest &request, HttpResponse &response) = 0;
    static bool isValidToken(QString token);
    template<typename Func>
    static auto invokeOnOwnerThread(QObject* owner, Func&& func) -> decltype(func())
    {
        using Result = decltype(func());
        if (QThread::currentThread() == owner->thread()) {
            return func();
        }
        Result result;
        QMetaObject::invokeMethod(owner, [&]() {
            result = func();
        }, Qt::BlockingQueuedConnection);
        return result;
    }

protected:
    void sendJsonError(HttpResponse& response, int statusCode, const QString& message);
    void invalidData(HttpResponse& response, const QString& description = "");
    void invalidToken(HttpResponse& response);
    void permissionDenied(HttpResponse& response, const QString& description = "");
    void notFound(HttpResponse& response, const QString& description = "");
    bool handleError(IResource::ResourceError error, HttpResponse& response);
    void handleModificationResult(IResource::ModificationResult& result, HttpResponse& response);
    void writeJsonSuccess(HttpResponse& response, const QVariant& data = QVariant());
    void writeVariant(const QVariant& variant, HttpResponse& response);

    IResourceHttpController::PathElements splitPath(const QString& path);
    QSharedPointer<IResource> getResource(const QString& path, const QString& type, const QString& token, HttpResponse &response);



private:
    bool _checkHttpTokens;
};

#endif // IRESOURCEHTTPCONTROLLER_H
