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
#ifndef _OBJECTIVE_NET_MESSAGE_HPP
#define _OBJECTIVE_NET_MESSAGE_HPP

#include "Core/CoreTypes.hpp"
#include "NetMessage.hpp"

enum
{
    _net_message_id_occupation_status_update,
    _net_message_id_objective_sync,
    _net_message_id_objective_working_sync,
    _net_message_id_objective_outloc_sync
};

#ifdef MSVC
#pragma pack(1)
#endif

class ObjectiveOccupationUpdate : public NetMessage
{
private:
    ObjectiveID objective_id;
    PlayerID player_id;

public:
    ObjectiveOccupationUpdate()
    {
        message_class = _net_message_class_objective;
        message_id = _net_message_id_occupation_status_update;
    }

    void set(ObjectiveID id, PlayerID player_id)
    {
        objective_id = ObjectiveID_toPortable(id);
        this->player_id = player_id;
    }

    ObjectiveID getObjectiveId() const
    {
        return ObjectiveID_fromPortable(objective_id);
    }

    PlayerID getPlayerId() const
    {
        return player_id;
    }
}__attribute__((packed));

class ObjectiveSyncData
{
public:
    PlayerID player_id;

    void set(PlayerID player_id)
    {
        this->player_id = player_id;
    }

    PlayerID getPlayerId() const
    {
        return player_id;
    }

}__attribute__((packed));

class ObjectiveSyncMesg : public NetMessage
{
private:
    ObjectiveID objective_id;

public:
    ObjectiveSyncData sync_data;

    ObjectiveSyncMesg()
    {
        message_class = _net_message_class_objective;
        message_id = _net_message_id_objective_sync;
    }

    void set(ObjectiveID id)
    {
        this->objective_id = ObjectiveID_toPortable(id);
    }
    
    ObjectiveID getObjectiveId() const
    {
        return ObjectiveID_fromPortable(objective_id);
    }

} __attribute__((packed));

#ifdef MSVC
#pragma pack()
#endif

#endif // ** _OBJECTIVE_NET_MESSAGE_HPP
