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


#include "SDL.h"

#include "PlayerNameView.hpp"
#include "Views/GameViewGlobals.hpp"
#include "Interfaces/GameConfig.hpp"
#include "Interfaces/StrManager.hpp"
#include "Views/Components/InputField.hpp"
#include "Views/Components/Label.hpp"

enum { BORDER_SPACE           =  4 };    
// irc nick is 16 chars max, 2 = "np", 1 spare for digit if nick is taken
enum { INPUT_FIELD_CHARACTERS = 15 };

// PlayerNameView
//---------------------------------------------------------------------------
PlayerNameView::PlayerNameView() : View()
{
    setSearchName("PlayerNameView");

    setAllowMove(false);

    moveTo(bodyTextRect.getLocation());

    int CHAR_XPIX = 8; // XXX hardcoded :-/
    resizeClientArea((INPUT_FIELD_CHARACTERS+1) * CHAR_XPIX + 8 + BORDER_SPACE * 2 + 200, 32 );

    init();
} // end PlayerNameView::PlayerNameView

// init
//---------------------------------------------------------------------------
void PlayerNameView::init()
{
    add( new Label(0,6, _("Player Name"), Color::white) );
    
    playerName = new InputField();
    playerName->setLocation(iXY(150, 2));
    playerName->setSize(getWidth()-160, 16);
    playerName->setMaxTextLength(INPUT_FIELD_CHARACTERS * 2); // XXX *2 for testing only
    playerName->setExcludedChars("\\");
    playerName->setText(*GameConfig::player_name);

    add(playerName);
} // end PlayerNameView::init
