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



#ifndef OBJECTCONTROLLER_H
#define OBJECTCONTROLLER_H

#include "httprequesthandler.h"
#include <QObject>
#include "Server/Resources/ResourceManager/ResourceManager.h"
#include "Server/Resources/ObjectResource/ObjectResource.h"
#include "Server/Authentication/AuthentificationService.h"
#include "Server/Authentication/User.h"
#include "IResourceHttpController.h"

using namespace stefanfrings;
class ObjectController : public IResourceHttpController
{
    Q_OBJECT
    Q_DISABLE_COPY(ObjectController)

public:
    /** Constructor */
    ObjectController();
    ~ObjectController(){}
    void handleResourceOperation(QString token, PathElements& pathElements, QString command, QVariantMap parameters, HttpRequest &request, HttpResponse &response);


};

#endif // OBJECTCONTROLLER_H
