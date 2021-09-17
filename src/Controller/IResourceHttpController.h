
/* Copyright (C) Friedemann Metzger - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Friedemann Metzger <friedemann.metzger@gmx.de>, 2017
*/

#ifndef IRESOURCEHTTPCONTROLLER_H
#define IRESOURCEHTTPCONTROLLER_H

#include <QObject>
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

    /** Constructor */
    IResourceHttpController(bool checkToken = true);

    /** Generates the response */
    void service(HttpRequest& request, HttpResponse& response);
    virtual void handleResourceOperation(QString token, PathElements& pathElements, QString command, QVariantMap parameters, HttpRequest &request, HttpResponse &response) = 0;

protected:
    void invalidData(HttpResponse& response, QString description = "");
    void invalidToken(HttpResponse& response);
    void permissionDenied(HttpResponse& response, QString description = "");
    bool handleError(IResource::ResourceError& error, HttpResponse& response, bool last = true);
    void handleMofidicationResult(IResource::ModificationResult& result, HttpResponse& response, bool last = true);
    void writeVariant(QVariant& variant, HttpResponse& response, bool isLast = true);
    IResourceHttpController::PathElements splitPath(QString path);
    QString preparePath(PathElements &elements, User* user);
    QSharedPointer<IResource> getResource(QString path, QString type, QString token, HttpResponse &response);

private:
    bool _checkHttpTokens;
};

#endif // IRESOURCEHTTPCONTROLLER_H
