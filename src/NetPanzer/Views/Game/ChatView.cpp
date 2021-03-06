/*
Copyright (C) 2012 Netpanzer Team. (www.netpanzer.org), Laurent Jacques

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

#include "Views/Game/ChatView.hpp"
#include "Classes/WorldInputCmdProcessor.hpp"
#include "Classes/ScreenSurface.hpp"
#include "Interfaces/KeyboardInterface.hpp"
#include "Interfaces/ChatInterface.hpp"
#include "Views/Theme.hpp"
#include "Views/Components/View.hpp"
#include "2D/Components/Button.hpp"
#include "Views/Components/tChatBox.hpp"
#include "2D/Components/InputField.hpp"
#include "2D/Palette.hpp"

#include "Actions/Action.hpp"

#ifdef WIN32
#include <windows.h>
#endif

#include "Actions/Action.hpp"

//#define CommandMax 13
//
//std::string CommandList[CommandMax] = {
//    "/server listcommands", "/server adminlogin", "/server unitspawnlist", "/server unitprofiles",
//    "/server listprofiles", "/server kick", "/server baselimit", "/server gamepass",
//    "/server map ", "/server autokick", "/server say", "/server listplayers", "/server flagtimer"}; 

//    case SDLK_TAB: {
//            cursorPos = 0;
//            strcpy(inputString, CommandList[commandPos].c_str());
//            commandPos++;
//            if (commandPos >= CommandMax) commandPos = 0;
//            cursorPos = strlen(inputString);
//        }

class SwitchChatModeAction : public Action
{
public:
    ChatView * view;
    SwitchChatModeAction(ChatView * view) : Action(false), view(view) {}
    void execute()
    {
        view->switchChatMode();
    }
};

class MinimizeChatAction : public Action
{
public:
    ChatView * view;
    MinimizeChatAction(ChatView * view) : Action(false), view(view) {}
    void execute()
    {
        view->minimizeChat();
    }
};

class RetoreChatAction : public Action
{
public:
    ChatView * view;
    RetoreChatAction(ChatView * view) : Action(false), view(view) {}
    void execute()
    {
        view->restoreChat();
    }
};

ChatView::ChatView() : GameTemplateView()
{
    setSearchName("ChatView");

    setAllowMove(true);
    resize(500, 150);
    moveTo(screen->getWidth()-getWidth(), screen->getHeight()-getHeight());

//    Surface button_images;
//    button_images.loadPNG(itScroll);
    
//    bHideWindow = createTitlebarButton( 0, 0, button_images, new MinimizeChatAction(this));
    add(bHideWindow);

//    Surface show_button_image;
//    show_button_image.grab(button_images, iRect(30,0, 15, 15) );
//    bShowWindow = createTitlebarButton( 15, 0, show_button_image, new RetoreChatAction(this));
//    bShowWindow->disable();
    add(bShowWindow);
    
    HideWindow = false;

    iRect r(0, 15, getWidth()-15, getHeight()-17);
    ChatList = new tChatBox(r);
    ChatList->setColor(0);
    ChatList->setAutoScroll(true);
    add(ChatList);
    
    input = new InputField();
    input->setLocation(2 + 16,getHeight()-16);
    input->setSize(getWidth()-(5 + 17),16);
    input->setMaxTextLength(500);
    input->setExcludedChars(UString("\\�`�"));
    
    add(input);
    
    switchModeButton = new Button(); //Button::createTextButton(">", iXY(0, getSizeY()-16), 14, 0);
    
    switchModeButton->setLocation(1, getHeight()-16);
    switchModeButton->setSize(16, 16);
    switchModeButton->setLabel(">");
    switchModeButton->setStateOffset(Button::BPRESSED, 1, 1);
//    switchModeButton->setAction(new SwitchChatModeAction(this));
    
    switchModeButton->setTextColors(Color::gray, Color::yellow, Color::gray, Color::darkGray);
    
    allBg.create(16,16);
    allBg.FillRoundRect(allBg.getRect(), 3, Color::white);
    allBg.RoundRect(allBg.getRect(), 3, Color::white);
    
    friendsBg.create(16,16);
    friendsBg.FillRoundRect(friendsBg.getRect(), 3, Color::yellow);
    friendsBg.RoundRect(friendsBg.getRect(), 3, Color::yellow);

    add(switchModeButton);
    
    isAllMode = true;
    setChatFriends();
}

void ChatView::doDraw( Surface& dest )
{
//    dest.bltLookup(dest.getRect(), Palette::filterBrightness());
    dest.bltString(35, 4, "Chat View", ctTexteNormal);
    View::doDraw( dest );
}

void ChatView::processEvents()
{
//    if ( Desktop::getKeyboardFocusComponent() == input )
    if ( false )
    {
        if ( (KeyboardInterface::getKeyPressed(SDLK_RETURN) || KeyboardInterface::getKeyPressed(SDLK_KP_ENTER) ) )
        {
            if ( input->getText().length() != 0 )
            {
                // @todo fix this
                const NPString& msg = ""; //input->getText();

                // @todo removed commands
//                if ( msg[0] != '/' || ! ScriptManager::runUserCommand( msg.substr(1) ) )
                if ( msg[0] != '/' )
                {
                    if ( isAllMode )
                    {
                        ChatInterface::say(msg);
                    }
                    else
                    {
                        ChatInterface::teamsay(msg);
                    }
                }

                input->clearText();
            }
        }
    }
#ifdef WIN32
    else if ( (KeyboardInterface::getKeyState(SDLK_LCTRL) || KeyboardInterface::getKeyState(SDLK_RCTRL))
                  && (KeyboardInterface::getKeyPressed(SDLK_c) ))
    {
        std::string str = ChatList->getTextItem();
        if (str != "")
        {
            if(OpenClipboard(NULL))
            {
                int pos = str.find_first_of(':')+2;
                std::string chaine = str.substr(pos);
                HGLOBAL hText = GlobalAlloc(GMEM_DDESHARE, chaine.length()+1);
                char * pText = (char*)GlobalLock(hText);
                strcpy(pText, chaine.c_str());
                GlobalUnlock(hText);
                EmptyClipboard();
                SetClipboardData(CF_TEXT, hText);
                CloseClipboard();
            }
        }
    }
#endif
    else
    {
        COMMAND_PROCESSOR.process(false);
    }
}

void ChatView::minimizeChat()
{
    HideWindow = true;
    bHideWindow->disable();
    bShowWindow->enable();
    
//    if ( Desktop::getKeyboardFocusComponent() == input )
//    {
//        Desktop::setKeyboardFocusComponent(0);
//    }
    
    removeComponent(input);
    removeComponent(switchModeButton);

    resize(500, 17);
    moveTo(screen->getWidth()-getWidth(), screen->getHeight()-getHeight());
}

void ChatView::restoreChat()
{
    HideWindow = false;
    bHideWindow->enable();
    bShowWindow->disable();

    resize(500, 150);
    moveTo(screen->getWidth()-getWidth(), screen->getHeight()-getHeight());
    
    add(switchModeButton);
    add(input);
}

//void ChatView::postMessage(PIX msgcolor, bool hasFlag, FlagID flag, const char *format, ...)
//{
//    char temp_str[256];
//    va_list vap;
//
//    va_start( vap, format );
//    vsnprintf( temp_str, 256, format, vap );
//    va_end( vap );
//
//    ChatList->AddChat(temp_str, msgcolor, hasFlag, flag);
//}

void ChatView::onDesktopResized(const iXY& oldResolution, const iXY& newResolution)
{
    (void)oldResolution;
    moveTo(newResolution.x-getWidth(), newResolution.y-getHeight());
}

void ChatView::clear()
{
    ChatList->Clear();
}

void ChatView::setChatFriends()
{
    if ( isAllMode )
    {
        isAllMode = false;
        PtrArray<Surface> bg(1);
        Surface *s = new Surface();
        s->copy(friendsBg);
        bg.push_back(s);
        switchModeButton->setImage(bg);
        input->setTextColor(Color::yellow);
    }
}

void ChatView::setChatAll()
{
    if ( ! isAllMode )
    {
        isAllMode = true;
        PtrArray<Surface> bg(1);
        Surface *s = new Surface();
        s->copy(allBg);
        bg.push_back(s);
        switchModeButton->setImage(bg);
        input->setTextColor(Color::white);
    }
}

//void ChatView::openChat()
//{
//    if ( HideWindow )
//    {
//        restoreChat();
//    }
//    
////    Desktop::setKeyboardFocusComponent(input);
//}

//void ChatView::openFriendsChat()
//{
//    if ( HideWindow )
//    {
//        restoreChat();
//    }
//    
//    setChatFriends();
//    
////    Desktop::setKeyboardFocusComponent(input);
//}

void ChatView::switchChatMode()
{
    if ( isAllMode )
    {
        setChatFriends();
    }
    else
    {
        setChatAll();
    }
}
