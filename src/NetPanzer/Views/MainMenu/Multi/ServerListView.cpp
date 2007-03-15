/*
Copyright (C) 2004 by Matthias Braun <matze@braunis.de>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <config.h>

#include "ServerListView.hpp"

#include <iostream>
#include <sstream>
#include "IPAddressView.hpp"
#include "GameViewGlobals.hpp"
#include "MasterServer/ServerInfo.hpp"
#include "Core/NetworkGlobals.hpp"

ServerListView* serverlistview = 0;

ServerListView::ServerListView()
    : queryThread(0)
{
    setSearchName("ServerListView");
    setTitle("Servers");
    setSubTitle("");

    moveTo(bodyTextRect.min + iXY(0, 170));
    resizeClientArea(bodyTextRect.max - bodyTextRect.min - iXY(0,170));

    setAllowResize(false);
    setAllowMove(false);
    setVisible(false);

    addButtonCenterText(iXY(getClientRect().getSizeX()-80,
                getClientRect().getSizeY() - Surface::getFontHeight() * 2),
            80, "Refresh", "", buttonRefresh);
    
    // XXX ugly
    serverlistview = this;
}

ServerListView::~ServerListView()
{
    delete queryThread;
    serverlistview = 0;
}

void
ServerListView::refresh()
{
    if(queryThread && queryThread->isRunning())
        return;
    
    delete queryThread;
   
    // don't clear before the delete or after the new, as the thread contains
    // pointers to the serverlist
    SDL_mutexP(serverlist.mutex);
    for(std::vector<masterserver::ServerInfo*>::iterator i = serverlist.begin();
            i != serverlist.end(); ++i)
        delete *i;
    serverlist.clear();
    SDL_mutexV(serverlist.mutex);

    queryThread = new masterserver::ServerQueryThread(&serverlist);   
}

void
ServerListView::buttonRefresh()
{
    serverlistview->refresh();
}

void
ServerListView::doDraw(Surface& windowArea, Surface& clientArea)
{
    clientArea.fill(Color::black);

    if(serverlist.empty()) {
        const char* msg = queryThread->getStateMessage();
        clientArea.bltString(iXY(0, 0), msg, Color::white);
        View::doDraw(windowArea, clientArea);
        return;
    }

    SDL_mutexP(serverlist.mutex);
    int y = 0;
    for(std::vector<masterserver::ServerInfo*>::iterator i = serverlist.begin();
            i != serverlist.end(); ++i) {
        const masterserver::ServerInfo& server = *(*i);

        if(server.status == masterserver::ServerInfo::QUERYING) {
            clientArea.bltString(iXY(0, y), server.address, Color::gray);
            clientArea.bltString(iXY(140, y), "querying...", Color::gray);
        } else if(server.status == masterserver::ServerInfo::TIMEOUT) {
            clientArea.bltString(iXY(0, y), server.address, Color::gray);
            clientArea.bltString(iXY(140, y), "timeout", Color::gray);
        } else if(server.protocol < NETPANZER_PROTOCOL_VERSION) {
            clientArea.bltString(iXY(0, y), server.address, Color::gray);
            clientArea.bltString(iXY(140, y), "server protocol too old", Color::gray);
        } else if(server.protocol > NETPANZER_PROTOCOL_VERSION) {
            clientArea.bltString(iXY(0, y), server.address, Color::gray);
            clientArea.bltString(iXY(140, y), "server protocol too new", Color::gray);
        } else {
            std::stringstream playerstr;
            playerstr << server.players << "/" << server.maxplayers;

            std::stringstream pingstr;
            pingstr << server.ping;
            
            std::stringstream servaddr;
            servaddr << server.address << ':' << server.port;
            
            uint8_t textcolor = Color::white;
            
            if (servaddr.str()==IPAddressView::szServer.getString())
                textcolor = Color::yellow;

            clientArea.bltString(iXY(0, y), server.name, textcolor);
            clientArea.bltString(iXY(350, y), playerstr.str(), textcolor);
            clientArea.bltString(iXY(400, y), server.map, textcolor);
            clientArea.bltString(iXY(550, y), pingstr.str(), textcolor);

        }

        y += Surface::getFontHeight();
        if(y >= clientArea.getPixY())
            break;                             
    }
    SDL_mutexV(serverlist.mutex);

    View::doDraw(windowArea, clientArea);
}

int
ServerListView::lMouseUp(const iXY& down_pos, const iXY& up_pos)
{
    if(down_pos.x < 0 || down_pos.y < 0 || up_pos.x < 0 || up_pos.y < 0)
        return View::lMouseUp(down_pos, up_pos);

    int listpos = down_pos.y / Surface::getFontHeight();
    if(listpos >= int(serverlist.size()) || 
            serverlist[listpos]->status != masterserver::ServerInfo::RUNNING)
        return View::lMouseUp(down_pos, up_pos);

    const masterserver::ServerInfo& server = *(serverlist[listpos]);
    std::stringstream addr;
    addr << server.address << ':' << server.port;
    IPAddressView::szServer.setString(addr.str());
    
    return View::lMouseUp(down_pos, up_pos);
}

