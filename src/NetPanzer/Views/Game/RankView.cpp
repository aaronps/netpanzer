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

#include "Resources/ResourceManager.hpp"

#include <vector>
#include <algorithm>

#include "Util/Exception.hpp"
#include "RankView.hpp"
#include "Views/GameViewGlobals.hpp"
#include "Classes/ScreenSurface.hpp"
#include "Interfaces/PlayerInterface.hpp"
#include "Interfaces/StrManager.hpp"
#include "Classes/Network/PlayerNetMessage.hpp"
#include "Classes/Network/NetworkClient.hpp"
#include "Interfaces/GameConfig.hpp"
#include "Objectives/ObjectiveInterface.hpp"
#include "Classes/WorldInputCmdProcessor.hpp"

#include "2D/Components/Label.hpp"
#include "Network/PlayerRequests/BreakAllianceRequest.hpp"
#include "Network/PlayerRequests/AllianceRequest.hpp"

#define HEADER_HEIGHT 24
#define ENTRY_HEIGHT 20

#define TABLE_BORDER 2
#define TABLE_BORDER_START (HEADER_HEIGHT+TABLE_BORDER)
#define TABLE_START (HEADER_HEIGHT + TABLE_BORDER)

#define ALLY_START 4
#define FLAG_START 26
// characters * character width
#define NAME_START (6*8)

#define TABLE_HEADER_PIX_LEN (52*8)

#define WINDOW_WIDTH (TABLE_HEADER_PIX_LEN + (TABLE_BORDER * 2 ) + 14+2)

char table_header[256];// =
//        "      Name                 Frags Deaths Points Objs.";

static const char * stats_format = "%-20s%6i%7i%7i%6i";

// RankView
//---------------------------------------------------------------------------
RankView::RankView() : GameTemplateView()
{
    setSearchName("RankView");
//    setTitle(_("Rankings"));

    moveTo(gameconfig->gameinterface.getRankPositionX(), gameconfig->gameinterface.getRankPositionY());
    resize(iXY(WINDOW_WIDTH, 200));
    checkArea(iXY(screen->getWidth(),screen->getHeight()));

    allyImage.loadPNG("pics/default/ally.png");
    allyRequestImage.loadPNG("pics/default/allyRequest.png");
    allyOtherImage.loadPNG("pics/default/allyOther.png");
    noAllyImage.loadPNG("pics/default/noAlly.png");
    colorImage.loadPNG("pics/default/playerColor.png");

    selected_line = -1;
    snprintf(table_header, sizeof(table_header), "    %-20s %6s %7s %7s %6s", 
             _("Name"),
             _("Frags"),
             _("Deaths"),
             _("Points"),
             _("Objs"));
} // end RankView::RankView

// doDraw
//---------------------------------------------------------------------------
void RankView::doDraw( Surface& dest )
{
    unsigned int flagHeight = ResourceManager::getFlag(0)->getHeight();
    unsigned int newheight = HEADER_HEIGHT
                             + (TABLE_BORDER * 2)
                             + (ENTRY_HEIGHT * PlayerInterface::countPlayers());

    if ( newheight != (unsigned int)getHeight() )
    {
        resize(WINDOW_WIDTH, newheight);
        return; // this frame draws nothing
    }
    
    bltViewBackground(dest);

//    dest.drawButtonBorder(
//            iRect(0, TABLE_BORDER_START, dest.getWidth()-1, dest.getHeight()-1),
//            Color::gray64, Color::white);

    dest.bltStringShadowed(0, 16, table_header, Color::red, Color::gray64);
    drawPlayerStats(dest, flagHeight);

    View::doDraw( dest);
} // end doDraw

class StatesSortByPoints
    : public std::binary_function<const PlayerState*, const PlayerState*, bool>
{
public:
    bool operator() (const PlayerState* state1, const PlayerState* state2) {
        return state1->getTotal() > state2->getTotal();
    }
};

class StatesSortByObjectives
    : public std::binary_function<const PlayerState*, const PlayerState*, bool>
{
public:
    bool operator() (const PlayerState* state1, const PlayerState* state2) {
        int p1 = state1->getObjectivesHeld();
        int p2 = state2->getObjectivesHeld();
        return p1 > p2 || (p1 == p2 && state1->getTotal() > state2->getTotal() );
    }
};


// drawPlayerStats
//---------------------------------------------------------------------------
// Purpose:
//---------------------------------------------------------------------------
void RankView::drawPlayerStats(Surface &dest, unsigned int flagHeight)
{
    char statBuf[256];

    states.clear();
    PlayerID i;
    for( i = 0; i < PlayerInterface::getMaxPlayers(); ++i)
    {
        PlayerState* state = PlayerInterface::getPlayer(i);
        if ( state->isActive() )
        {
            states.push_back(state);
        }
    }

    switch(GameConfig::game_gametype)
    {
        case _gametype_objective:
            std::sort(states.begin(), states.end(), StatesSortByObjectives());
            break;
        case _gametype_timelimit:
        case _gametype_fraglimit:
            std::sort(states.begin(), states.end(), StatesSortByPoints());
            break;
    }

    int cur_line_pos = TABLE_START + ((ENTRY_HEIGHT - Surface::getFontHeight())/2);
    int flag_pos = TABLE_START + (int(ENTRY_HEIGHT - flagHeight))/2;
    Surface * flag = 0;
    int cur_state = 0;
    for(std::vector<const PlayerState*>::iterator i = states.begin();
            i != states.end(); ++i) {
        const PlayerState* state = *i;

        snprintf(statBuf, sizeof(statBuf),
                stats_format, state->getName().substr(0,20).c_str(),
                state->getKills(), state->getLosses(), state->getTotal(),
                state->getObjectivesHeld());
        dest.bltStringShadowed(NAME_START, cur_line_pos, statBuf,
                               (cur_state == selected_line)?Color::yellow:Color::gray224,
                               Color::gray64);
        
        flag = ResourceManager::getFlag(state->getFlag());
        flag->blt( dest, FLAG_START, flag_pos ); // full blit
        if ( state->getID() != PlayerInterface::getLocalPlayerIndex() )
        {
            // XXX ALLY
            bool meWithHim = PlayerInterface::isSingleAllied(PlayerInterface::getLocalPlayerIndex(), state->getID());
            bool himWithMe = PlayerInterface::isSingleAllied(state->getID(), PlayerInterface::getLocalPlayerIndex());            
            if ( meWithHim && himWithMe )
            {
                allyImage.bltTrans(dest, ALLY_START, flag_pos ); // blit full
            }
            else if ( meWithHim )
            {
                allyRequestImage.bltTrans(dest, ALLY_START, flag_pos ); // blit full
            }
            else if ( himWithMe )
            {
                allyOtherImage.bltTrans(dest, ALLY_START, flag_pos ); // blit full
            }
            else
            {
                noAllyImage.bltTrans(dest, ALLY_START, flag_pos ); // blit full
            }
        }

        colorImage.bltTransColor(dest, TABLE_HEADER_PIX_LEN+2, flag_pos, state->getColor());

        cur_line_pos += ENTRY_HEIGHT;
        flag_pos += ENTRY_HEIGHT;
        ++cur_state;
    }

} // end RankView::drawPlayerStats

void RankView::notifyMoveTo()
{
    gameconfig->gameinterface.setRankPositionX(getLocationX());
    gameconfig->gameinterface.setRankPositionY(getLocationY());
}

void RankView::lMouseDown(const iXY& pos)
{
    GameTemplateView::lMouseDown(pos);
    if (GameConfig::game_teammode) return;
    // XXX ALLY
    if ( pos.x >= 4 && pos.x <= 24 && pos.y >= TABLE_START )
    {
        unsigned int ypos = pos.y - TABLE_START;
        unsigned int linepos = ypos / ENTRY_HEIGHT;
        if ( linepos < states.size() )
        {
            unsigned int destplayer = states[linepos]->getID();
            unsigned int localplayer = PlayerInterface::getLocalPlayerIndex();
            if ( destplayer != localplayer )
            {
                if ( PlayerInterface::isSingleAllied(localplayer, destplayer) )
                {
                    BreakAllianceRequest req;
                    req.with_player_id = destplayer;
                    CLIENT->sendMessage(&req, sizeof(req));
                }
                else
                {
                    AllianceRequest req;
                    req.with_player_id = destplayer;
                    CLIENT->sendMessage(&req, sizeof(req));
                }
            }
        }
    }
}

void RankView::mouseMove(const iXY & prevPos, const iXY &newPos)
{
    GameTemplateView::mouseMove(prevPos, newPos);
    selected_line = -1;
    if ( newPos.y >= TABLE_START )
    {
        selected_line = (newPos.y-TABLE_START) / ENTRY_HEIGHT;
    }
}

void RankView::doActivate()
{
    selected_line = -1;
}

void RankView::doDeactivate()
{
    selected_line = -1;
}

void RankView::processEvents()
{
    COMMAND_PROCESSOR.process(false);
}
