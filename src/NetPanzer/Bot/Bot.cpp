/*
Copyright (C) 2003 Ivo Danihelka <ivo@danihelka.net>

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

#include "Bot.hpp"

#include "PlacementMatrix.hpp"
#include "TerminalNetMesg.hpp"
#include "Client.hpp"
#include "ObjectiveInterface.hpp"
#include "UnitBase.hpp"
#include "NetworkState.hpp"
#include "Objective.hpp"
#include "Util/Log.hpp"

Bot *Bot::s_bot = 0;
//-----------------------------------------------------------------
void
Bot::initialize(Bot *bot)
{
    s_bot = bot;
}
//-----------------------------------------------------------------
void
Bot::shutdown()
{
    if (s_bot) {
        delete s_bot;
        s_bot = 0;
    }
}
//-----------------------------------------------------------------
void
Bot::moveUnit(UnitBase *unit, iXY map_pos)
{
    assert(unit != 0);

    PlacementMatrix matrix;
    matrix.reset(map_pos);
    matrix.getNextEmptyLoc(&map_pos);

    TerminalUnitCmdRequest comm_mesg;
    comm_mesg.comm_request.setHeader(unit->unit_id,
            _umesg_flag_unique);
    comm_mesg.comm_request.setMoveToLoc(map_pos);

    CLIENT->sendMessage(&comm_mesg, sizeof(TerminalUnitCmdRequest), 0);
    m_tasks.setUnitTask(unit, BotTaskList::TASK_MOVE);

    LOGGER.debug("bot: moveUnit %d to %dx%d",
            unit->unit_id.index, map_pos.x, map_pos.y);
}
//-----------------------------------------------------------------
void
Bot::attackUnit(UnitBase *unit, UnitBase *enemyUnit)
{
    assert(unit != 0);
    assert(enemyUnit != 0);

    TerminalUnitCmdRequest comm_mesg;
    comm_mesg.comm_request.setHeader(unit->unit_id, _umesg_flag_unique);
    comm_mesg.comm_request.setTargetUnit(enemyUnit->unit_id);

    CLIENT->sendMessage(&comm_mesg, sizeof(TerminalUnitCmdRequest), 0);
    m_tasks.setUnitTask(unit, BotTaskList::TASK_ATTACK);

    LOGGER.debug("bot: attackUnit %d to %d",
            unit->unit_id.index, enemyUnit->unit_id.index);
}
//-----------------------------------------------------------------
void
Bot::manualFire(UnitBase *unit, iXY world_pos)
{
    assert(unit != 0);

    TerminalUnitCmdRequest comm_mesg;
    comm_mesg.comm_request.setHeader(unit->unit_id, _umesg_flag_unique);
    comm_mesg.comm_request.setManualFire(world_pos);

    CLIENT->sendMessage(&comm_mesg, sizeof(TerminalUnitCmdRequest), 0);
    //NOTE: manual fire is not special unit task,
    // unit can move and fire simultanous
}
//-----------------------------------------------------------------
void
Bot::produceUnit(int outpostID, int selectedProduce)
{
    assert(selectedProduce <= _unit_type_humvee);

    // Send the server the selected unit and whether factory power is on.
    TerminalOutpostUnitGenRequest term_mesg;

    term_mesg.unit_gen_request.set(outpostID, selectedProduce, true);

    CLIENT->sendMessage(&term_mesg, sizeof(TerminalOutpostUnitGenRequest), 0);

    // XXX is this needed?
    if (NetworkState::status == _network_state_client) {
        ObjectiveInterface::sendMessage(&(term_mesg.unit_gen_request));
    }
    LOGGER.debug("bot: produceUnit outpost=%d selectedProduce=%d",
            outpostID, selectedProduce);
}
