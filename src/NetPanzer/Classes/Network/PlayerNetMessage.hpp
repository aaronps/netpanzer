/*
Copyright (C) 1998 Pyrosoft Inc. (www.pyrosoftgames.com), Matthew Bogue

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
#ifndef _PLAYERNETMESSAGE_HPP
#define _PLAYERNETMESSAGE_HPP

#include "Core/CoreTypes.hpp"
#include "NetMessage.hpp"

enum { _net_message_id_player_connect_id,
       _net_message_id_player_sync_flag,
       _net_message_id_player_update_flag,
       _net_message_id_player_sync_state,
       _net_message_id_player_score_update,
       _net_message_id_player_alliance_request,
       _net_message_id_player_alliance_update,
       _net_message_id_player_flagtimer_update,
       _net_message_id_player_changeteam_request,
       _net_message_id_player_ready_request,
       _net_message_id_player_team_score_sync,
       _net_message_id_player_vote_request,
       _net_message_id_player_vote
     };

#ifdef MSVC
#pragma pack(1)
#endif

class PlayerConnectID : public NetMessage
{
public:
    PlayerID player_id;

    PlayerConnectID(PlayerID id)
        : player_id(id)
    {
        message_class = _net_message_class_player;
        message_id = _net_message_id_player_connect_id;
    }
}
__attribute__((packed));


class PlayerStateSync : public NetMessage
{
public:
    NetworkPlayerState player_state;

    PlayerStateSync()
    {
        message_class = _net_message_class_player;
        message_id = _net_message_id_player_sync_state;
    }

    PlayerStateSync(NetworkPlayerState state)
        : player_state(state)
    {
        message_class = _net_message_class_player;
        message_id = _net_message_id_player_sync_state;
    }
}
__attribute__((packed));

class PlayerFlagSync : public NetMessage
{
public:
    PlayerID player_id;
    Uint8 player_flag[FLAG_WIDTH*FLAG_HEIGHT];

    PlayerFlagSync()
    {
        message_class = _net_message_class_player;
        message_id = _net_message_id_player_sync_flag;
    }
}
__attribute__((packed));

class UpdatePlayerFlag : public NetMessage
{
public:
    Uint8 player_flag[FLAG_WIDTH*FLAG_HEIGHT];

    UpdatePlayerFlag()
    {
        message_class = _net_message_class_player;
        message_id = _net_message_id_player_update_flag;
    }
}
__attribute__((packed));

class PlayerScoreUpdate : public NetMessage
{
private:
    PlayerID kill_by_player_index;
    PlayerID kill_on_player_index;
public:
    Uint8  unit_type;

    PlayerScoreUpdate()
    {
        message_class = _net_message_class_player;
        message_id = _net_message_id_player_score_update;
    }

    void set(PlayerID kill_by_index, PlayerID kill_on_index, Uint8 unit_type)
    {
        kill_by_player_index = kill_by_index;
        kill_on_player_index = kill_on_index;
        this->unit_type = unit_type;
    }

    void setKillByPlayerIndex(PlayerID kill_by_index)
    {
        kill_by_player_index = kill_by_index;
    }
    PlayerID getKillByPlayerIndex() const
    {
        return kill_by_player_index;
    }

    void setKillOnPlayerIndex(PlayerID kill_on_index)
    {
        kill_on_player_index = kill_on_index;
    }
    PlayerID getKillOnPlayerIndex() const
    {
        return kill_on_player_index;
    }
}
__attribute__((packed));

enum { _player_make_alliance,
       _player_break_alliance
     };

class PlayerAllianceRequest : public NetMessage
{
private:
    PlayerID allie_by_player_index;
    PlayerID allie_with_player_index;
public:
    Uint8  alliance_request_type;

    PlayerAllianceRequest()
    {
        message_class = _net_message_class_player;
        message_id = _net_message_id_player_alliance_request;
    }

    void set(PlayerID allie_by_player_index, PlayerID allie_with_player_index,
            Uint8 alliance_request_type)
    {
        this->allie_by_player_index = allie_by_player_index;
        this->allie_with_player_index = allie_with_player_index;
        this->alliance_request_type = alliance_request_type;
    }
    PlayerID getAllieByPlayerIndex() const
    {
        return allie_by_player_index;
    }
    PlayerID getAllieWithPlayerIndex() const
    {
        return allie_with_player_index;
    }
}
__attribute__((packed));


class PlayerAllianceUpdate : public NetMessage
{
private:
    PlayerID allie_by_player_index;
    PlayerID allie_with_player_index;
public:
    Uint8  alliance_update_type;

    PlayerAllianceUpdate()
    {
        message_class = _net_message_class_player;
        message_id = _net_message_id_player_alliance_update;
    }

    void set(PlayerID by_player_index, PlayerID with_player_index,
            Uint8 update_type )
    {
        allie_by_player_index = by_player_index;
        allie_with_player_index = with_player_index;
        alliance_update_type = update_type;
    }

    Uint16 getAllieByPlayerIndex() const
    {
        return allie_by_player_index;
    }
    Uint16 getAllieWithPlayerIndex() const
    {
        return allie_with_player_index;
    }
}
__attribute__((packed));

class PlayerFlagTimerUpdate : public NetMessage
{
public:
    Uint16 flag_timer;

    PlayerFlagTimerUpdate(Uint16 flagtimer)
    {
        message_class = _net_message_class_player;
        message_id = _net_message_id_player_flagtimer_update;
        flag_timer = flagtimer;
    }

    Uint16 getflagtimer() const
    {
        return flag_timer;
    }
}
__attribute__((packed));

enum { change_team_request,
       change_team_Accepted
     };

class PlayerTeamRequest : public NetMessage
{
private:
    PlayerID player_index;
    Uint8 team_index;
public:
    Uint8  request_type;

    PlayerTeamRequest()
    {
        message_class = _net_message_class_player;
        message_id = _net_message_id_player_changeteam_request;
    }

    void set(PlayerID player_idx, Uint8 team_idx, Uint8 type_request)
    {
        this->player_index = player_idx;
        this->team_index = team_idx;
        this->request_type = type_request;
    }
    PlayerID getPlayerIndex() const
    {
        return player_index;
    }
    Uint8 gettoteamindex() const
    {
        return team_index;
    }
}
__attribute__((packed));

enum { ready_request,
       ready_Accepted};

class PlayerReadyRequest : public NetMessage
{
private:
    PlayerID player_index;
public:
    Uint8  request_type;

    PlayerReadyRequest()
    {
        message_class = _net_message_class_player;
        message_id = _net_message_id_player_ready_request;
    }

    void set(PlayerID player_idx, Uint8 type_request)
    {
        this->player_index = player_idx;
        this->request_type = type_request;
    }
    PlayerID getPlayerIndex() const
    {
        return player_index;
    }
}
__attribute__((packed));

class TeamScoreSync : public NetMessage
{
private:
    short TeamKills;
    short TeamLosses;
public:
    Uint8 TeamID;

    TeamScoreSync()
    {
        message_class = _net_message_class_player;
        message_id = _net_message_id_player_team_score_sync;
    }

    void set(Uint8 Team_ID, short kills, short Losses)
    {
        TeamKills = kills;
        TeamLosses =  Losses;
        TeamID = Team_ID;
    }

    void setKills(short kills)
    {
        TeamKills = kills;
    }
    short getKills() const
    {
        return TeamKills;
    }

    void setLosses(short Losses)
    {
        TeamLosses = Losses;
    }
    short getLosses() const
    {
        return TeamLosses;
    }
}
__attribute__((packed));

class PlayerVoteRequested : public NetMessage
{
public:
    Uint8  vote_type;

    PlayerVoteRequested()
    {
        message_class = _net_message_class_player;
        message_id = _net_message_id_player_vote_request;
    }

    void set(Uint8 vote_type)
    {
        this->vote_type = vote_type;
    }
}
__attribute__((packed));

class PlayerSendVote : public NetMessage
{
private:
    PlayerID player_index;
public:
    Uint8 player_vote;

    PlayerSendVote()
    {
        message_class = _net_message_class_player;
        message_id = _net_message_id_player_vote;
    }

    void set(PlayerID player_idx, bool player_vote)
    {
        this->player_index = player_idx;
        this->player_vote = player_vote;
    }

    PlayerID getPlayerIndex() const
    {
        return player_index;
    }
}
__attribute__((packed));

#ifdef MSVC
#pragma pack()
#endif

#endif // ** _PLAYERNETMESSAGE_HPP
