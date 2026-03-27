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


#include "LoginController.h"
#include "Server/Authentication/AuthentificationService.h"
#include "Server/Authentication/User.h"
#include "httpsessionstore.h"
#include <QJsonDocument>
#include <QJsonObject>

extern HttpSessionStore* sessionStore;

LoginController::LoginController()
{
}

void LoginController::service(HttpRequest &request, HttpResponse &response)
{
    response.setHeader("Content-Type", "application/json; charset=UTF-8");
    HttpSession session = sessionStore->getSession(request, response);

    QByteArray method = request.getMethod();

    // POST /logout
    QString path = QString::fromLatin1(request.getPath());
    if (path.contains("logout", Qt::CaseInsensitive)) {
        QString token = session.get("token").toString();
        if (!token.isEmpty()) {
            AuthenticationService::instance()->logout(token);
            session.remove("token");
        }
        QJsonObject obj;
        obj["success"] = true;
        response.setStatus(200, "OK");
        response.write(QJsonDocument(obj).toJson(QJsonDocument::Compact), true);
        return;
    }

    // Check if already logged in
    QString existingToken = session.get("token").toString();
    if (!existingToken.isEmpty() && AuthenticationService::instance()->validateToken(existingToken)) {
        QJsonObject obj;
        obj["token"] = existingToken;
        response.setStatus(200, "OK");
        response.write(QJsonDocument(obj).toJson(QJsonDocument::Compact), true);
        return;
    }

    // Parse credentials from JSON body or query parameters
    QString userName;
    QString password;

    QJsonParseError parseError;
    QJsonDocument bodyJson = QJsonDocument::fromJson(request.getBody(), &parseError);
    if (!bodyJson.isEmpty() && parseError.error == QJsonParseError::NoError) {
        QJsonObject bodyObj = bodyJson.object();
        userName = bodyObj["user"].toString();
        password = bodyObj["pass"].toString();
    }

    if (userName.isEmpty())
        userName = request.getParameter("user");
    if (password.isEmpty())
        password = request.getParameter("pass");

    if (userName.isEmpty() || password.isEmpty()) {
        QJsonObject obj;
        obj["error"] = true;
        obj["code"] = 400;
        obj["message"] = QString("Missing credentials");
        response.setStatus(400, "Bad Request");
        response.write(QJsonDocument(obj).toJson(QJsonDocument::Compact), true);
        return;
    }

    AuthenticationService::ErrorCode errCode;
    QString token = AuthenticationService::instance()->login(userName, password, &errCode);
    if (errCode == AuthenticationService::NoError) {
        session.set("token", token);
        QJsonObject obj;
        obj["token"] = token;
        response.setStatus(200, "OK");
        response.write(QJsonDocument(obj).toJson(QJsonDocument::Compact), true);
        return;
    }

    QJsonObject obj;
    obj["error"] = true;
    int statusCode = 401;
    QString message;
    switch (errCode) {
    case AuthenticationService::IncorrectPassword:
        message = "Incorrect password";
        break;
    case AuthenticationService::UserNotExists:
        message = "User not found";
        break;
    case AuthenticationService::PermissionDenied:
        message = "Permission denied";
        statusCode = 403;
        break;
    default:
        message = "Login failed";
        break;
    }
    obj["code"] = statusCode;
    obj["message"] = message;
    response.setStatus(statusCode, message.toLatin1());
    response.write(QJsonDocument(obj).toJson(QJsonDocument::Compact), true);
}
