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
 
 
#include <iostream>
#include <string>
 
#include <time.h>
 
#include "Util/Log.hpp"
#include "Util/Exception.hpp"
#include "Util/FileSystem.hpp"
#include "Util/NTimer.hpp"
#include "SDLVideo.hpp"
#include <stdlib.h>
 
#include "Interfaces/ConsoleInterface.hpp"
#include "Interfaces/StrManager.hpp"
#include "2D/Color.hpp"
#include "2D/Palette.hpp"

#include "libpng-1.6.5/png.h"
 
#ifdef _WIN32
#include "Interfaces/GameConfig.hpp"
#endif
 
#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION "testing"
#endif
 
SDLVideo* Screen; // get rid of this later...
 
static int best_bpp = 0;
 
SDLVideo::SDLVideo()
    : frontBuffer(0)
{
#ifdef _WIN32
    if ( gameconfig->video.useDirectX() )
    {
        putenv("SDL_VIDEODRIVER=directx");
    }
#endif
    if(SDL_InitSubSystem(SDL_INIT_VIDEO))
    {
        throw Exception("Couldn't initialize SDL_video subsystem: %s",
                        SDL_GetError());
    }
 
    if ( ! best_bpp )
    {
        const SDL_VideoInfo *v = SDL_GetVideoInfo();
        if ( v )
        {
            best_bpp = v->vfmt->BitsPerPixel;
        }
    }
 
    // XXX unfortunately SDL initializes the keyboard again :-/
    SDL_EnableUNICODE(1);
}
 
SDLVideo::~SDLVideo()
{
    if ( frontBuffer )
    {
        SDL_FreeSurface(frontBuffer);
    }
 
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}
 
static bool getNearestFullScreenMode(int flags, int* width, int* height)
{
    SDL_Rect** modes = SDL_ListModes(0, flags);
    if ( modes == 0 )
    {
        return false;
    }
    else if ( modes != (SDL_Rect**)-1 )
    {
        unsigned int min_x_dif = -1;
        unsigned int min_y_dif = -1;
        unsigned int nearest = 0;
        for ( int n = 0; modes[n]; ++n )
        {
            unsigned int new_x_dif = abs((*width) - modes[n]->w);
 
            if ( new_x_dif <= min_x_dif )
            {
                unsigned int new_y_dif = abs((*height) - modes[n]->h);
                if ( new_y_dif <= min_y_dif )
                {
                    nearest = n;
                    min_x_dif = new_x_dif;
                    min_y_dif = new_y_dif;
                }
            }
        }
        *width = modes[nearest]->w;
        *height = modes[nearest]->h;
    }
 
    return true;
}
 
 
void SDLVideo::setVideoMode(int width, int height, int bpp, Uint32 flags)
{
    if ( frontBuffer )
    {
        SDL_FreeSurface(frontBuffer);
    }
 
    int new_width = width;
    int new_height = height;
 
    if ( flags&SDL_FULLSCREEN )
    {
        //bpp = best_bpp;
        getNearestFullScreenMode(flags, &new_width, &new_height);
        LOGGER.warning("Setting fullscreen mode %d x %d (original %d x %d)",
                       new_width, new_height, width, height);
    }
 
    flags |= SDL_HWPALETTE;
 
    frontBuffer = SDL_SetVideoMode(new_width, new_height, bpp, flags);
    if(!frontBuffer)
    {
        throw Exception("Couldn't set display mode (%dx%d, %X): %s",
                        new_width, new_height, flags, SDL_GetError());
    }
 
    // let's scare the mouse :)
    SDL_ShowCursor(SDL_DISABLE);
    // and set a window title
    SDL_WM_SetCaption("NetPanzer " PACKAGE_VERSION, 0);
}
 
bool SDLVideo::isDisplayModeAvailable(int width, int height, int bpp,
                                      Uint32 flags)
{
    flags |= SDL_HWPALETTE; // SDL_ANYFORMAT
    int res = SDL_VideoModeOK(width, height, bpp, flags);
 
    LOGGER.warning("Mode %dx%dx%d %s is %s", width, height, bpp,
                   (flags&SDL_FULLSCREEN)?"full screen":"windowed",
                   res?"available":"NOT available");
 
    return SDL_VideoModeOK(width, height, bpp, flags);
}
 
void SDLVideo::lockDoubleBuffer(unsigned char **buffer)
{
    if(SDL_MUSTLOCK(frontBuffer) && SDL_LockSurface(frontBuffer))
        throw Exception("Couldn't lock display buffer");
 
    *buffer = (unsigned char *)frontBuffer->pixels;
}
 
void SDLVideo::unlockDoubleBuffer()
{
    if(SDL_MUSTLOCK(frontBuffer))
        SDL_UnlockSurface(frontBuffer);
}
 
void SDLVideo::copyDoubleBufferandFlip()
{
    SDL_UpdateRect(frontBuffer, 0, 0, 0, 0);
}
 
void SDLVideo::setPalette(SDL_Color *color)
{
    SDL_SetColors(frontBuffer, color, 0, 256);
}
 
SDL_Surface* SDLVideo::getSurface()
{
    return frontBuffer;
}
 
void SDLVideo::doScreenshoot()
{
    static NTimer timer(1000);
 
    if ( ! timer.isTimeOut() )
    {
        return;
    }
 
    filesystem::mkdir("screenshoots");
 
    char buf[256];
    time_t curtime = time(0);
    struct tm* loctime = localtime(&curtime);
    strftime(buf, sizeof(buf), "screenshoots/%Y%m%d_%H%M%S.png", loctime);
    std::string realfile = filesystem::getRealWriteName(buf);
        
    if ( SDL_MUSTLOCK(frontBuffer) )
    {
        SDL_LockSurface(frontBuffer);
    }
    
    png_image pim = {0};
    pim.version = PNG_IMAGE_VERSION;
    
    pim.width = frontBuffer->w;
    pim.height = frontBuffer->h;
    pim.format = PNG_FORMAT_FLAG_COLOR | PNG_FORMAT_FLAG_COLORMAP | PNG_FORMAT_FLAG_ALPHA;
    pim.colormap_entries = 256;
    
    const int stride = frontBuffer->pitch - frontBuffer->w;

    // little hack to not copy palette
    Palette::color[0].nothing = 0xff;
    png_image_write_to_file(&pim, realfile.c_str(), false, frontBuffer->pixels, stride, &Palette::color);
    Palette::color[0].nothing = 0;
    
    if ( SDL_MUSTLOCK(frontBuffer) )
    {
        SDL_UnlockSurface(frontBuffer);
    }
    
    LOGGER.info("Screenshoot saved as: %s", realfile.c_str());
//    ConsoleInterface::postMessage(Color::cyan, false, 0, _("Screenshoot saved as: %s"), buf);
    timer.reset();
}
 
 