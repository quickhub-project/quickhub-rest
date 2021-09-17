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


#ifndef LISTCONTROLLER_H
#define LISTCONTROLLER_H

#include "httprequesthandler.h"
#include "IResourceHttpController.h"
#include <QObject>

using namespace stefanfrings;
class ListResource;
class ListController : public IResourceHttpController
{
    Q_OBJECT
    Q_DISABLE_COPY(ListController)

public:
    /** Constructor */
    ListController();
    ~ListController(){}

    /** Generates the response */
    void handleResourceOperation(QString token, PathElements& pathElements, QString command, QVariantMap parameters,HttpRequest &request, HttpResponse &response);

};

#endif // LISTCONTROLLER_H
