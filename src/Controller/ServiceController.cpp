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

#include "ServiceController.h"
#include "Server/Services/ServiceManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QEventLoop>
#include <QTimer>
#include <QUuid>

ServiceController::ServiceController() : IResourceHttpController()
{
}

void ServiceController::handleResourceOperation(QString token, PathElements& pathElements, QVariantMap parameters, HttpRequest &request, HttpResponse &response)
{
    QByteArray method = request.getMethod();
    QString path = QString::fromLatin1(request.getPath());
    QStringList segments = path.split("/", Qt::SkipEmptyParts);

    // GET /services - list all services
    if (method == "GET" && segments.count() == 1) {
        QMap<QString, IService*> services = ServiceManager::instance()->getServices();
        QJsonArray serviceList;
        QMapIterator<QString, IService*> it(services);
        while (it.hasNext()) {
            it.next();
            QJsonObject svc;
            svc["name"] = it.key();
            svc["methods"] = QJsonArray::fromStringList(it.value()->getServiceCalls());
            serviceList.append(svc);
        }
        response.setStatus(200, "OK");
        response.write(QJsonDocument(serviceList).toJson(QJsonDocument::Compact), true);
        return;
    }

    // GET /services/{name} - service info
    if (method == "GET" && segments.count() == 2) {
        QString serviceName = segments[1];
        IService* service = ServiceManager::instance()->service(serviceName);
        if (!service) {
            notFound(response, "Service not found: " + serviceName);
            return;
        }
        QJsonObject obj;
        obj["name"] = serviceName;
        obj["methods"] = QJsonArray::fromStringList(service->getServiceCalls());
        response.setStatus(200, "OK");
        response.write(QJsonDocument(obj).toJson(QJsonDocument::Compact), true);
        return;
    }

    // POST /services/{name}/{method} - call service method
    if (method == "POST" && segments.count() >= 3) {
        QString serviceName = segments[1];
        QString methodName = segments[2];

        IService* service = ServiceManager::instance()->service(serviceName);
        if (!service) {
            notFound(response, "Service not found: " + serviceName);
            return;
        }

        QString cbID = QUuid::createUuid().toString(QUuid::WithoutBraces);
        QVariant argument = QVariant::fromValue(parameters);

        QEventLoop loop;
        QVariant result;
        bool responded = false;

        auto conn = QObject::connect(service, &IService::response, &loop,
            [&](const QString& uid, const QVariant& answer) {
                if (uid == cbID) {
                    result = answer;
                    responded = true;
                    loop.quit();
                }
            });

        QTimer timer;
        timer.setSingleShot(true);
        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(30000);

        bool accepted = invokeOnResourceThread(service, [&]() {
            return service->call(methodName, token, cbID, argument);
        });
        if (!accepted) {
            QObject::disconnect(conn);
            notFound(response, "Unknown method: " + methodName);
            return;
        }

        if (!responded) {
            loop.exec();
        }
        QObject::disconnect(conn);


        if (!responded) {
            sendJsonError(response, 504, "Service call timed out");
            return;
        }

        if(!result.isValid()){
            sendJsonError(response, 502, "Service call returned invalid data");
        }

        writeVariant(result, response);
        return;
    }

    sendJsonError(response, 405, "Method not allowed");
}
