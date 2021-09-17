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


#ifndef HTTPSERVERPLUGIN_H
#define HTTPSERVERPLUGIN_H

#include "IPlugin.h"
#include <QObject>

class HttpServerPlugin : public IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPlugin_iid FILE "httpServerPlugin.json")
    Q_INTERFACES(IPlugin)

public:
    HttpServerPlugin(QObject* parent = 0);
    virtual bool init(QVariantMap parameters);
    virtual bool shutdown();
    virtual QString getPluginName();
};

#endif // HTTPSERVERPLUGIN_H
