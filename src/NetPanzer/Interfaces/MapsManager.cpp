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
#include <config.h>

#include <stdio.h>
#include <string.h>
#include <memory>

#include "MapsManager.hpp"
#include "GameConfig.hpp"

/// removes leading and trailing whitespaces from a string
static inline std::string trim(const std::string& str)
{
    using std::string;
    
    string::size_type start = str.find_first_not_of(" \t\n\r");
    string::size_type end = str.find_last_not_of(" \t\n\r");

    return string(str, start, end-start+1);
}

std::string MapsManager::getNextMap(const std::string& map)
{
    using std::string;
    std::cout << "NextMap from '" << map << "': ";

    // parse the mapcycle in the config
    const string& mapcycle = gameconfig->mapcycle;
    string::size_type i = mapcycle.find(',', 0);
    string::size_type lasti = 0;
    if(i == string::npos)
        i = mapcycle.size() - 1;
    string currentMap = trim(string(mapcycle, 0, i));
    string firstmap = currentMap;

    bool takeNext = false;
    do {
        if(takeNext) {
            std::cout << currentMap << "\n";
            return currentMap;
        }
        if(currentMap == map)
            takeNext = true;
        
        if(i == mapcycle.size())
            break;
        lasti = i;
        i = mapcycle.find(',', i+1);
        if(i == string::npos) {
            i = mapcycle.size();
        }
        std::cout << "check: '" << string(mapcycle, lasti+1, i-lasti+1) << "'\n";
        std::cout << "c2: '" << trim(string(mapcycle, lasti+1, i-lasti+1)) <<
                "'\n";
        currentMap = trim(string(mapcycle, lasti+1, i-lasti+1));
    } while(1);

    std::cout << firstmap << "(first) \n";
    return firstmap;
}

