/*
Copyright (C) 2004 by Matthias Braun <matze@braunis.de>
 
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


#include "StringUtil.hpp"

void string_to_params( const std::string& str, std::vector<std::string>& parameters )
{
    parameters.clear();

    static const char* limiters[] = { ", \t", "\"" };

    std::string::size_type start = 0;
    std::string::size_type end = 0;

    do
    {
        start = str.find_first_not_of(" \t", start);
        if ( start != std::string::npos )
        {
            int limit_index = 0;
            if ( str[start] == '"' && start < str.length()-1 )
            {
                limit_index = 1;
                ++start;
            }

            end = str.find_first_of(limiters[limit_index], start);

            parameters.push_back( str.substr(start, end != std::string::npos ? end-start : end) );

            if ( end != std::string::npos )
            {
                start = str.find_first_not_of(limiters[limit_index], end);
            }
        }
    } while ( start != std::string::npos && end != std::string::npos );
}
