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

#include "Core/NetworkGlobals.hpp"
#include "Interfaces/StrManager.hpp"
#include <cstddef>

struct version_data
{
    int protocol;
    const char * text;
};

struct version_data older_versions[] =
{
    (struct version_data){ 1019, "0.8.2" },
    (struct version_data){ 1030, "0.8.3" },
    (struct version_data){ 1105, "0.8.4" } // this is valid for future
};

#define NUM_OLD_VERSIONS (sizeof(older_versions)/sizeof(version_data))

const char * getNetpanzerProtocolMessage(const int protocol)
{
    for ( size_t n = 0; n < NUM_OLD_VERSIONS; n++)
    {
        if ( older_versions[n].protocol == protocol )
        {
            char old[128];
            snprintf(old, sizeof(old),_("This server requires older netpanzer %s"), older_versions[n].text);

            return old;
        }
    }

    if ( protocol < older_versions[0].protocol )
    {
        return _("The server is a very old netpanzer version, cannot use");
    }

    if ( protocol < older_versions[NUM_OLD_VERSIONS-1].protocol )
    {
        return _("This server is an older development version, cannot use");
    }

    return _("This server is a newer netpanzer, please check http://www.netpanzer.org");
}
