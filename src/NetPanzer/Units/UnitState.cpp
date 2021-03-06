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

#include "Util/Endian.hpp"
#include "Units/UnitState.hpp"

UnitState::UnitState()
{
    body_angle.set( 0, 10 );
    turret_angle.set( 0, 10 );

    // ** NOTE: TEMPORARY UNTIL UNIT PROPERITIES CODED

    // TODO: the bounds shall be fixed, maybe add some offset
//    bbox.setBounds( iXY( -25, -15 ), iXY( 25, 35 ) );
    bbox.setSize(iXY(25,25));

    select = false;
    orientation = 0;
    speed_rate = 20;
    speed_factor = 2;

    hit_points = 100;
    damage_factor = 30;
    weapon_range = 409600;
    defend_range = 100;
    reload_time = 45;

    threat_level = _threat_level_all_clear;
}

int UnitState::percentDamageInt( void )
{
    int damage_percentage;

    damage_percentage = (int)  ( (1 - ( ( (float) hit_points ) / ( (float) max_hit_points) ) ) * 100 );

    return( damage_percentage );
}

float UnitState::percentDamageFloat( void )
{
    float damage_percentage;

    damage_percentage = ( (1 - ( ( (float) hit_points ) / ( (float) max_hit_points) ) ) * 100 );

    return( damage_percentage );
}
