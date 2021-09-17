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


/** Storage for session cookies */
extern HttpSessionStore* sessionStore;


LoginController::LoginController()
{

}

void LoginController::service(HttpRequest &request, HttpResponse &response)
{
    QString userName = request.getParameter("user");
    QString password = request.getParameter("pass");


    HttpSession session=sessionStore->getSession(request,response);
    QString token = session.get("token").toString();
    if(!token.isEmpty() && AuthenticationService::instance()->validateToken(token))
    {
        response.write("You are logged in!");
        return;
    }

    if(token.isEmpty() && userName.isEmpty() && password.isEmpty())
    {
        response.write("Missing parameters.");
        return;
    }

    AuthenticationService::ErrorCode errCode;
    token = AuthenticationService::instance()->login(userName, password, &errCode);
    if(errCode == AuthenticationService::NoError)
    {
        session.set("token", token);

        if(session.contains("initial"))
        {
            QString redirectPath = session.get("initial").toString();
            response.redirect(redirectPath.toLatin1());
            return;
        }
        else
        {
            response.write(token.toLatin1(), true);
            return;
        }
    }
    else
    {
        qCritical()<<Q_FUNC_INFO<<"  "<<errCode;
    }
}
