/*
Copyright (C) 2003 by Matthias Braun
 
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
#ifndef __UI_DRAWING_FUNCTIONS_HPP__
#define __UI_DRAWING_FUNCTIONS_HPP__

#include <SDL/SDL.h>

#include "Types/iXY.hpp"
#include "Types/iRect.hpp"
#include "Color.hpp"

namespace UI
{
    void drawLine(SDL_Surface* surface, iXY from, iXY to, Color color);
    void drawRect(SDL_Surface* surface, iRect rect, Color color);
    void fillRect(SDL_Surface* surface, iRect rect, Color color);
} 

#endif