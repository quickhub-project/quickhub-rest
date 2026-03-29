/* This file is part of the REST Plugin for the QuickHub framework
 * (git@github.com:quickhub-project/quickhub-rest.git).
 * Copyright (c) 2021 Friedemann Metzger - www.quickhub.org
 *  * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *  * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *  * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */



#include "IResourceHttpController.h"
#include <QJsonDocument>
#include <QJsonObject>

#include "Server/Resources/ResourceManager/ResourceManager.h"
#include "Server/Authentication/AuthentificationService.h"
#include "Server/Authentication/User.h"
#include "httpsessionstore.h"

extern HttpSessionStore* sessionStore;

IResourceHttpController::IResourceHttpController(bool checkToken):
    _checkHttpTokens(checkToken)
{
}

void IResourceHttpController::service(HttpRequest &request, HttpResponse &response)
{
    response.setHeader("Content-Type", "application/json; charset=UTF-8");

    HttpSession session = sessionStore->getSession(request, response);
    QMultiMap<QByteArray, QByteArray> params = request.getParameterMap();
    QString token;
    QString path = QString::fromLatin1(request.getPath());

    // 1. Authorization: Bearer <token>
    QString authHeader = QString::fromLatin1(request.getHeader("Authorization"));
    if (authHeader.startsWith("Bearer ", Qt::CaseInsensitive)) {
        token = authHeader.mid(7).trimmed();
    }

    // 2. Session cookie fallback
    if (token.isEmpty() && session.contains("token")) {
        token = session.get("token").toString();
    }

    // 3. Query parameter fallback
    if (token.isEmpty()) {
        token = QString::fromLatin1(params.value("token"));
    }

    if (token.isEmpty() && _checkHttpTokens) {
        invalidToken(response);
        return;
    }

    if (path.endsWith('/')) {
        path.chop(1);
    }

    params.remove("token");

    if (_checkHttpTokens) {
        bool valid = isValidToken(token);
        if (!valid) {
            invalidToken(response);
            return;
        }
    }

    QVariantMap parameters;
    QJsonParseError error;
    QByteArray body = request.getBody();
    if (!body.isEmpty()) {
        QJsonDocument bodyJson = QJsonDocument::fromJson(body, &error);
        if (error.error != QJsonParseError::NoError) {
            sendJsonError(response, 400, "Invalid JSON: " + error.errorString());
            return;
        }
        parameters = bodyJson.toVariant().toMap();
    } else {
        QMultiMapIterator<QByteArray, QByteArray> it(params);
        while (it.hasNext()) {
            it.next();
            parameters.insert(QString::fromLatin1(it.key()), QString::fromLatin1(it.value()));
        }
    }

    PathElements elements = splitPath(path);
    handleResourceOperation(token, elements, parameters, request, response);
}

IResourceHttpController::PathElements IResourceHttpController::splitPath(const QString& path)
{
    PathElements result;
    QStringList tokens = path.split("/", Qt::SkipEmptyParts);

    if (tokens.count() < 2) {
        result.valid = false;
        return result;
    }

    result.type = tokens[0];
    result.resource = tokens[1].replace(".", "/");
    if (tokens.count() > 2) {
        result.id = tokens[2];
        result.path = result.resource + "/" + result.id;

        for (int i = 3; i < tokens.count(); i++) {
            result.additionalElements.append(tokens[i]);
        }
    }

    result.valid = true;
    return result;
}

void IResourceHttpController::sendJsonError(HttpResponse &response, int statusCode, const QString &message)
{
    QJsonObject obj;
    obj["error"] = true;
    obj["code"] = statusCode;
    obj["message"] = message;
    response.setStatus(statusCode, message.toLatin1());
    response.write(QJsonDocument(obj).toJson(QJsonDocument::Compact), true);
}

void IResourceHttpController::invalidData(HttpResponse &response, const QString& description)
{
    QString msg = "Invalid data / Insufficient arguments";
    if (!description.isEmpty())
        msg += ": " + description;
    sendJsonError(response, 400, msg);
}

void IResourceHttpController::invalidToken(HttpResponse &response)
{
    sendJsonError(response, 403, "Invalid token. Please log in and try again.");
}

void IResourceHttpController::permissionDenied(HttpResponse &response, const QString& description)
{
    QString msg = "Permission denied";
    if (!description.isEmpty())
        msg += ": " + description;
    sendJsonError(response, 403, msg);
}

void IResourceHttpController::notFound(HttpResponse &response, const QString& description)
{
    QString msg = "Not found";
    if (!description.isEmpty())
        msg += ": " + description;
    sendJsonError(response, 404, msg);
}

bool IResourceHttpController::handleError(IResource::ResourceError error, HttpResponse& response)
{
    switch (error) {
    case IResource::NO_ERROR:
        return false;
    case IResource::INVALID_PARAMETERS:
        invalidData(response);
        return true;
    case IResource::PERMISSION_DENIED:
        permissionDenied(response);
        return true;
    case IResource::UNKNOWN_ITEM:
        notFound(response, "Unknown item");
        return true;
    default:
        sendJsonError(response, 500, "Internal server error");
        return true;
    }
}

void IResourceHttpController::handleModificationResult(IResource::ModificationResult &result, HttpResponse &response)
{
    if (!handleError(result.error, response)) {
        writeVariant(result.data, response);
    }
}

void IResourceHttpController::writeJsonSuccess(HttpResponse &response, const QVariant &data)
{
    QJsonObject obj;
    obj["error"] = false;
    if (data.isValid())
    {
        obj["data"] = QJsonValue::fromVariant(data);
    }
    response.setStatus(200, "OK");
    response.write(QJsonDocument(obj).toJson(QJsonDocument::Compact), true);
}

void IResourceHttpController::writeVariant(const QVariant &variant, HttpResponse &response)
{
    response.setStatus(200, "OK");
    auto responseData = QJsonDocument::fromVariant(variant).toJson(QJsonDocument::Compact);
    qDebug()<<responseData;
    response.write(responseData, true);
}

bool IResourceHttpController::isValidToken(QString token)
{
    return invokeOnOwnerThread(AuthenticationService::instance(), [&](){return AuthenticationService::instance()->isValidToken(token);});
}

resourcePtr IResourceHttpController::getResource(const QString& path, const QString& type, const QString& token, HttpResponse &response)
{
    Err::CloudError error;

    auto resourceManager = ResourceManager::instance();
    resourcePtr resource = invokeOnOwnerThread(resourceManager, [&](){return resourceManager->getOrCreateResource(type, path, token, &error);});

    if (error == Err::NO_ERROR)
    {
        return resource;
    }

    if (error == Err::INVALID_TOKEN)
    {
        invalidToken(response);
    }
    else if (error == Err::PERMISSION_DENIED)
    {
        permissionDenied(response);
    }
    else
    {
        sendJsonError(response, 500, "Could not access resource");
    }

    return resourcePtr();
}
