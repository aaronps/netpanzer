/*
Copyright (C) 2012 Netpanzer Team. (www.netpanzer.org), Laurent Jacques

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

#include "Interfaces/ConsoleInterface.hpp"
#include "Interfaces/GameConfig.hpp"
#include "Interfaces/MapInterface.hpp"
#include "Interfaces/TeamManager.hpp"
#include "Interfaces/Team.hpp"
#include "Interfaces/PlayerInterface.hpp"
#include "Classes/Network/NetworkClient.hpp"
#include "Classes/Network/PlayerNetMessage.hpp"
#include "Classes/Network/NetworkServer.hpp"
#include "Util/Log.hpp"
#include "Util/StringUtil.hpp"


Team       * TeamManager::Teams_lists = 0;
Uint8        TeamManager::max_Teams = 0;

void TeamManager::initialize(const Uint8 _max_teams)
{
    char txtBuf[256];
    max_Teams = _max_teams;
    int team_id;
    
    delete[] Teams_lists;
    Teams_lists = new Team[max_Teams];

    std::vector<NPString> plist;
    NPString pl = *GameConfig::game_team_names;
    string_to_params(pl, plist);

    for ( team_id = 0; team_id < max_Teams; ++team_id )
    {
        Teams_lists[ team_id ].initialize(team_id);
        if (team_id < (Uint8) plist.size()) Teams_lists[ team_id ].setName(plist[team_id]);
        snprintf(txtBuf, sizeof(txtBuf), "pics/default/team-%d.bmp", team_id+1);
        Teams_lists[ team_id ].loadFlag(txtBuf);
    }
}

void TeamManager::addPlayer(PlayerID player_id)
{
    int team_id;
    int lowTeam = 0, countPlayers = Teams_lists[ 0 ].countPlayers();

    for ( team_id = 0; team_id < max_Teams; ++team_id )
    {
        //LOGGER.warning("number of players %d in team %d", Teams_lists[ team_id ].countPlayers(), team_id);
        if (Teams_lists[ team_id ].countPlayers() < countPlayers)
        {
            countPlayers = Teams_lists[ team_id ].countPlayers();
            lowTeam = team_id;
        }
    }
    Teams_lists[ lowTeam ].addPlayer(player_id);
}

void TeamManager::addPlayerinTeam(PlayerID player_id, Uint8 team_id)
{
    Teams_lists[ team_id ].addPlayer(player_id);
}

void TeamManager::removePlayer(PlayerID player_id, Uint8 team_id)
{
    Teams_lists[ team_id ].removePlayer(player_id);
}

void TeamManager::cleanUp()
{
    Uint8 team_id;
    for ( team_id = 0; team_id < max_Teams; ++team_id )
    {
        Teams_lists[ team_id ].cleanUp();
    }
    delete[] Teams_lists;
    Teams_lists = 0;
    max_Teams = 0;
}

iXY TeamManager::getPlayerSpawnPoint(PlayerID player_id)
{
    Uint8 Team_id = PlayerInterface::getPlayer(player_id)->getTeamID();

    iXY spawn_point;
    switch (Team_id)
    {
    case 0:
        spawn_point = MapInterface::getMinSpawnPoint();
        break;
    case 1:
        spawn_point = MapInterface::getMaxSpawnPoint();
        break;
    default:
        spawn_point = MapInterface::getFreeSpawnPoint();
    }
    return spawn_point;
}

long TeamManager::getTeamScore(  Uint8 team_id )
{
    return Teams_lists[team_id].getTeamScore();
}

const std::string& TeamManager::getTeamName( Uint8 team_id )
{
    return Teams_lists[team_id].getName();
}

void TeamManager::drawFlag(Uint8 team_id, Surface &dest, int x, int y)
{
    Teams_lists[team_id].drawFlag(dest, x, y);
}

bool TeamManager::testRuleScoreLimit( long score_limit )
{
    for (Uint8 team_id = 0; team_id < max_Teams; ++team_id )
    {
        if ( Teams_lists[team_id].getTeamScore() >= score_limit )
            return( true );
    }
    return( false );
}

void TeamManager::PlayerrequestchangeTeam(PlayerID player_id, Uint8 newteam)
{
    PlayerTeamRequest Changeteam_request;
    Changeteam_request.set(player_id, newteam, change_team_request);
    CLIENT->sendMessage( &Changeteam_request, sizeof(PlayerTeamRequest));
}
 
void TeamManager::serverrequestchangeTeam(PlayerID player_id, Uint8 newteam)
{
    Uint8 current_team = PlayerInterface::getPlayer(player_id)->getTeamID();
 
    if ( (Teams_lists[newteam].countPlayers() < Teams_lists[current_team].countPlayers())
            && (Teams_lists[newteam].countPlayers() > 0))
    {
        Teams_lists[current_team].removePlayer(player_id);
        Teams_lists[newteam].addPlayer(player_id);
        PlayerTeamRequest Changeteam_request;
 
        Changeteam_request.set(player_id, newteam, change_team_Accepted);
        SERVER->broadcastMessage( &Changeteam_request, sizeof(PlayerTeamRequest));
    }
}
 
void TeamManager::PlayerchangeTeam(PlayerID player_id, Uint8 team_idx)
{
    Uint8 current_team = PlayerInterface::getPlayer(player_id)->getTeamID();
    Teams_lists[current_team].removePlayer(player_id);
    Teams_lists[team_idx].addPlayer(player_id);
    ConsoleInterface::postMessage(Color::yellow, false, 0,
                                  "%s has changed to team %s.",
                                  PlayerInterface::getPlayer(player_id)->getName().c_str(), 
                                  Teams_lists[current_team].getName().c_str());
}
 
 
 
 
 