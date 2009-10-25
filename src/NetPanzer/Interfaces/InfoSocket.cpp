/*
Copyright (C) 2007 by Aaron Perez <aaronps@gmail.com>

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



#include "InfoSocket.hpp"
#include "Util/StringTokenizer.hpp"

#include "Core/NetworkGlobals.hpp"
#include "Interfaces/GameConfig.hpp"
#include "Interfaces/PlayerInterface.hpp"
#include "Objectives/ObjectiveInterface.hpp"
#include "Interfaces/GameManager.hpp"
#include "Resources/ResourceManager.hpp"

#include "Util/Log.hpp"

#include <string>
#include <sstream>

using namespace std;
#define MAKE_STRING(X) #X

InfoSocket::InfoSocket(int p) : socket(0)
{
    Address addr = Address::resolve( gameconfig->bindaddress, p);
    socket = new network::UDPSocket(addr,this);
    
    // This parameters are fixed always
    // others I plan to be modificable while game is running.
    stringstream s;
    s << "gamename\\netpanzer\\protocol\\" << NETPANZER_PROTOCOL_VERSION
      << "\\hostname\\" << gameconfig->playername
      << "\\gameversion\\" << MAKE_STRING(PACKAGE_VERSION);
    statusHead = s.str();
}

InfoSocket::~InfoSocket()
{
    if (socket)
        socket->destroy();
    socket=0;
}

void
InfoSocket::onSocketError(UDPSocket *so, const char * msg)
{
    (void)so;
    LOGGER.debug("InfoSocket: socket error '%s'", msg);
}

void
InfoSocket::onDataReceived(UDPSocket *s, const Address &from, const char *data, const int len)
{
    (void)s;
    string querypacket(data,len);
    StringTokenizer qtokenizer(querypacket, '\\');
    
    LOGGER.info("Infosocket:: received packet from [%s] size [%d]", from.getIP().c_str(), len);

    string query;
    while ( !(query = qtokenizer.getNextToken()).empty())
    {
        LOGGER.debug("InfoSocket:: Received query '%s'", query.c_str());
        if ( query == "status" )
        {
            string answer = prepareStatusPacket();

            LOGGER.debug("InfoSocket:: sending answer [%s][%d]", answer.c_str(), (int)answer.size());
            socket->send(from, answer.c_str(), answer.size());

            break;
        }
        else if(query == "echo")
        {
            string echotoken = qtokenizer.getNextToken();
            LOGGER.info("InfoSocket:: Received echo query of size %lu", (unsigned long)echotoken.size());
            if ( echotoken.size() )
                socket->send(from, echotoken.c_str(), echotoken.size());
        }
        else if ( query == "getflag" )
        {
            string flagstr = qtokenizer.getNextToken();
            
            LOGGER.info("InfoSocket:: Received flag query for %s", flagstr.c_str());
            if ( flagstr.size() )
            {
                unsigned int flagnum = atoi(flagstr.c_str());
                if ( flagnum < 256 && ResourceManager::isFlagActive(flagnum) )
                {
                    string answer = prepareFlagPacket(flagnum);
                    socket->send(from, answer.c_str(), answer.size());
                }
            }
        }
    }
}

string
InfoSocket::prepareStatusPacket()
{
    stringstream s;
    int playingPlayers = PlayerInterface::countPlayers();
    int maxPlayers = PlayerInterface::getMaxPlayers();
    
    s << statusHead
      << "\\mapname\\"    << gameconfig->map
      << "\\mapcycle\\"   << gameconfig->mapcycle
      << "\\numplayers\\" << playingPlayers
      << "\\maxplayers\\" << maxPlayers;
    
    if ( !playingPlayers )
        s << "\\empty\\1";
    else if ( playingPlayers==maxPlayers )
        s << "\\full\\1";

    s << "\\gamestyle\\" << gameconfig->getGameTypeString()
      << "\\units_per_player\\" << gameconfig->GetUnitsPerPlayer()
      << "\\time\\" << GameManager::getGameTime()/60
      << "\\timelimit\\" << gameconfig->timelimit
      << "\\fraglimit\\" << gameconfig->fraglimit
      << "\\objectivelimit\\" << ObjectiveInterface::getObjectiveLimit();

    int n = 0;
    for(int i = 0; i < maxPlayers; ++i) {
        PlayerState* playerState = PlayerInterface::getPlayer(i);
        if(playerState->getStatus() != _player_state_active)
            continue;
        s << "\\player_" << n << "\\" << playerState->getName()
          << "\\kills_"  << n << "\\" << playerState->getKills()
          << "\\deaths_" << n << "\\" << playerState->getLosses()
          << "\\score_"  << n << "\\" << playerState->getObjectivesHeld()
          << "\\points_"  << n << "\\" << playerState->getTotal()
          << "\\flag_"   << n << "\\" << (int) playerState->getFlag()
          << "\\flagu_"   << n << "\\" << ResourceManager::getFlagUsedCount(playerState->getFlag());
        n++;
    }
    
    s << "\\final\\";

    return s.str();
}

static const char hextochar[] = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' };

string
InfoSocket::prepareFlagPacket(const int flagNum)
{
    Uint8 rawflag[20*14];
    ResourceManager::getFlagSyncData(flagNum, rawflag);

    unsigned char flagdata[((20*14)*2) + 1];
    flagdata[((20*14)*2)] = 0;

    int curpos = 0;
    for ( int y = 0; y < 14; ++y )
    {
        for ( int x = 0; x < 20; ++x )
        {
            unsigned char b = rawflag[(y*20)+x];
            flagdata[curpos] = hextochar[((b>>4)&0x0f)];
            ++curpos;
            flagdata[curpos] = hextochar[b&0x0f];
            ++curpos;
        }
    }
    
    stringstream s;
    s << "\\flag\\" << flagNum << '\\' << flagdata << '\\';
    return s.str();
}
