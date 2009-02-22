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
#ifndef _WORLDINPUTCMDPROCESSOR_HPP
#define _WORLDINPUTCMDPROCESSOR_HPP

#include "SelectionList.hpp"
#include "Interfaces/MouseInterface.hpp"
#include "Util/Timer.hpp"

class WorldInputCmdProcessor
{
protected:
    iRect world_win;

    bool left_button_hold_action_complete;

    bool selection_box_active;
    iXY box_press, box_release;

    ObjectiveID outpost_goal_selection; //number of the outpost, -1 if none
    iXY output_pos_press;

    bool right_mouse_scroll_moved;
    bool selectBoundBoxUnits();

    bool manual_fire_state;

    /// timer that detects double keypressed (for centering selected unit group)
    TimeStamp lastSelectTime;

    SelectionList selection_group_lists[10];
    SelectionList working_list;
    unsigned long current_selection_list_index;
    unsigned int current_selection_list_bits;
    SelectionList target_list;

    void setSelectionList(unsigned long new_list_index);
    void cycleSelectedUnits(unsigned long new_list_index);

    unsigned char getCursorStatus(const iXY& loc);
    void setMouseCursor(unsigned char world_cursor_status);

    void getManualControlStatus();

    void sendManualFireCommand(const iXY &world_pos);
    void sendMoveCommand(const iXY &world_pos);
    void sendAttackCommand(const iXY &world_pos);
    void sendAllianceRequest(const iXY &world_pos, bool make_break);

    void evalLeftMButtonEvents(const MouseEvent &event);
    void evalRightMButtonEvents(const MouseEvent &event);
    void evalMouseMoveEvents(const MouseEvent &event);

    void evaluateMouseEvents();

    unsigned long keyboard_input_mode;

    void keyboardInputModeCommand();

    long enter_key_hit_count;
    void setKeyboardInputModeChatMesg();
    void keyboardInputModeChatMesg();
    void setKeyboardInputModeAllieChatMesg();
    void keyboardInputModeAllieChatMesg();

    void evaluateKeyCommands();
    void evaluateGroupingKeys();

    void evaluateKeyboardEvents();

    void jumpLastAttackedUnit();
    /** move camera to a position that shows the currently selected group of
     * tank
     */
    void centerSelectedUnits();

    bool getConsoleInputString(char *input_string);

    static ObjectiveID selected_objective_id;

public:
    WorldInputCmdProcessor();

    void process();

    void inFocus();

    void updateScrollStatus(const iXY &mouse_pos);

    void draw();

    void closeSelectionBox();

    static bool isObjectiveSelected();
    static const char* getSelectedObjectiveName();
    static iXY getSelectedObjectiveWorldPos();            
};

extern WorldInputCmdProcessor COMMAND_PROCESSOR;

#endif // ** _WORLDINPUTCMDPROCESSOR_HPP
