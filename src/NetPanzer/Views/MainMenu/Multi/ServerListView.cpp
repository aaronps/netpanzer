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
    SDL_mutexP(serverlist.mutex);
    serverlist.clear();
    SDL_mutexV(serverlist.mutex);
    
    delete queryThread;
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
        clientArea.bltString(iXY(0, 0), "Searching for servers", Color::white);
        View::doDraw(windowArea, clientArea);
        return;
    }

    SDL_mutexP(serverlist.mutex);
    int y = 0;
    for(std::vector<masterserver::ServerInfo>::iterator i = serverlist.begin();
            i != serverlist.end(); ++i) {
        const masterserver::ServerInfo& server = *i;

        if(i->querying) {
            clientArea.bltString(iXY(0, y), server.address, Color::gray);
            clientArea.bltString(iXY(140, y), "querying...", Color::gray);
        } else {
            std::stringstream playerstr;
            playerstr << server.players << "/" << server.maxplayers;

            std::stringstream pingstr;
            pingstr << server.ping;
            
            clientArea.bltString(iXY(0, y), server.name, Color::white);
            clientArea.bltString(iXY(140, y), playerstr.str(), Color::white);
            clientArea.bltString(iXY(200, y), server.map, Color::white);
            clientArea.bltString(iXY(300, y), pingstr.str(), Color::white);
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
    if(listpos >= int(serverlist.size()) || serverlist[listpos].querying)
        return View::lMouseUp(down_pos, up_pos);

    const masterserver::ServerInfo& server = serverlist[listpos];
    IPAddressView::szServer.setString(server.address);
    
    return View::lMouseUp(down_pos, up_pos);
}

