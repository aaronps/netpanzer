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

#include <stdarg.h>
#include <stdio.h>
#include "ArrayTemplate.hpp"
#include "Timer.hpp"
#include "Point.hpp"
#include "Surface.hpp"


class ConsoleLine
 {
  public:
   boolean visible;
   PIX     color;
   char    string[256];
   Timer   life_timer;       
 };

typedef ArrayTemplate< ConsoleLine > ConsoleLineArray;

enum { _console_overlap, _console_full };

class ConsoleInterface 
 {
  protected:
  static FILE *con_file;
  static boolean log_file_active;
  static boolean stdout_pipe;

  static long console_size; 
  static ConsoleLineArray line_list;

  static PointXYi surface_size;
  static Recti    bounds;
  static PointXYi line_offset;
  static long     vertical_spacing;
  static long     horizontal_spacing;
  static long     max_char_per_line;

  static long line_index; 

  static boolean input_string_active;
  
  static int  maxCharCount;
  static char inputString[256];
  static char inputPrompt[256];
  static int  cursorPos;

  public:

  static void initialize( long size );

  static void setToSurfaceSize( PointXYi pix );

  static void openLogFile( void );
  static void closeLogFile( void );
  static void setStdoutPipe( boolean on_off );

  static void update( Surface &surface );
  static void update_overlap( Surface &surface );

  static void postMessage( PIX color, const char *format, ... ); 
  
  static void postMessage( const char *format, ... ); 
  
  static void printf( const char *format, ... );

  /* Input String Methods */
  static void setInputStringStatus( boolean on_off );
  static void resetInputString( char *prompt );
  static void addChar(int newChar);
  static void addExtendedChar(int newExtendedChar);
  static void getInputString( char *string );
    

 };



#endif // ** _CONSOLEINTERFACE_HPP
