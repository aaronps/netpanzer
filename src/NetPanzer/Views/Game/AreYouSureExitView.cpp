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


#include "AreYouSureExitView.hpp"
#include "Views/Components/Desktop.hpp"
#include "Views/Components/Button.hpp"
#include "2D/Palette.hpp"
#include "Classes/ScreenSurface.hpp"
#include "Interfaces/GameManager.hpp"
#include "Interfaces/StrManager.hpp"

#include "Actions/ActionManager.hpp"

// AreYouSureExitView
//---------------------------------------------------------------------------
AreYouSureExitView::AreYouSureExitView() : SpecialButtonView()
{
    setSearchName("AreYouSureExitView");
    loaded = false;
	} // end AreYouSureExitView::AreYouSureExitView

// init
//---------------------------------------------------------------------------
void AreYouSureExitView::init()
{
    moveTo(iXY(0, 0));
    resize(iXY(800, 480));

    int x = (getClientRect().getWidth() - (141 * 2 + 20)) / 2;
    int y = getClientRect().getHeight()/2 + 30;
    add( Button::createTextButton( _("YES"), iXY(x, y), 137, ActionManager::getAction("quit")));
    x += 141 + 10;
    add( Button::createTextButton( _("NO"), iXY(x, y), 137, ActionManager::getAction("hide_confirmexit")));
    loaded = true;
} // end AreYouSureExitView::init

// doDraw
//---------------------------------------------------------------------------
void AreYouSureExitView::doDraw( Surface& dest )
{
    dest.bltLookup(getClientRect(), Palette::darkGray256.getColorArray());
    //viewArea.drawButtonBorder(r, Color::lightGreen, Color::darkGreen);

    dest.bltStringCenter(_("Are you sure you wish to exit netPanzer?"), Color::white);

    View::doDraw( dest );
} // end AreYouSureExitView::doDraw

// doActivate
//---------------------------------------------------------------------------
void AreYouSureExitView::doActivate()
{
    if ( ! loaded )
    {
	init();
    }
    Desktop::setActiveView(this);

} // end AreYouSureExitView::doActivate
