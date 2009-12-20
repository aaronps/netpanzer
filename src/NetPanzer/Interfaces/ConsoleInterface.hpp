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
#ifndef _CONSOLEINTERFACE_HPP
#define _CONSOLEINTERFACE_HPP

#include <vector>
#include <stdarg.h>
#include <stdio.h>
#include "Core/CoreTypes.hpp"
#include "Util/Timer.hpp"
#include "2D/Surface.hpp"
#include "Util/Log.hpp"


class ConsoleLine
{
public:
    bool    visible;
    IntColor     color;
    bool    hasFlag;
    FlagID  flag;
    char    string[256];
    Timer   life_timer;
    ConsoleLine() : visible(false) {}
};

enum { _console_overlap, _console_full };

class ConsoleInterface
{
protected:
    static bool stdout_pipe;

    static long console_size;
    static std::vector<ConsoleLine> line_list;

    static iXY surface_size;
    static iRect    bounds;
    static iXY line_offset;
    static long     vertical_spacing;
    static long     horizontal_spacing;
    static long     max_char_per_line;

    static long line_index;

    static bool input_string_active;

    static int  maxCharCount;
    static char inputString[256];
    static char inputPrompt[256];
    static int  cursorPos;

public:
    static void initialize( long size );

    static void setToSurfaceSize( iXY pix );

    static void setStdoutPipe( bool on_off );

    static void update( Surface &surface );
    static void update_overlap( Surface &surface );

    static void postMessage( IntColor msgColor, bool hasFlag, FlagID flag, const char *format, ... )
        __attribute__(( format(printf, 4, 5) ));

    static void post( IntColor msgColor, bool hasFlag, FlagID flag, const char *msg )
    {
        postMessage( msgColor, hasFlag ,flag, "%s", msg);
    }

    /* Input String Methods */
    static void setInputStringStatus( bool on_off );
    static void resetInputString( char *prompt );
    static void addChar(int newChar);
    static void addExtendedChar(int newExtendedChar);
    static void getInputString( char *string );
};

#endif // ** _CONSOLEINTERFACE_HPP
