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

#include <string.h>
#include <ctype.h>
#include "2D/Color.hpp"
#include "Util/Exception.hpp"
#include "cInputField.hpp"
#include "Util/TimerInterface.hpp"
#include "KeyboardInterface.hpp"
#include "Util/Log.hpp"

////////////////////////////////////////////////////////////////////////////
// cInputFieldString definitions.
////////////////////////////////////////////////////////////////////////////

// init
//--------------------------------------------------------------------------
void cInputFieldString::init(const char *string, int maxCharCount, int maxWidth)
{
    cInputFieldString::maxCharCount = maxCharCount;
    cInputFieldString::maxWidth = maxWidth;
    assert(maxCharCount > 0);
    assert(maxWidth > 0);

    cInputFieldString::string = new char [maxCharCount + 1];

    if (strlen(string) > 0 && strlen(string) < (size_t) maxCharCount) {
        strcpy(cInputFieldString::string, string);
    } else {
        cInputFieldString::string[0] = '\0';
    }

} // end init

// setString
void cInputFieldString::setString(const std::string& string)
{
    // XXX notify cinputfield to resetString positions
    strncpy(this->string, string.c_str(), maxCharCount);
} // end setString

// reset
//--------------------------------------------------------------------------
void cInputFieldString::reset()
{
    string       = 0;
    maxCharCount = 0;
} // end reset


////////////////////////////////////////////////////////////////////////////
// cInputField definitions.
////////////////////////////////////////////////////////////////////////////

cInputField::cInputField()
{
    reset();
}

cInputField::cInputField(const cInputField& other)
    : pos(other.pos), maxCharCount(other.maxCharCount),
    strDisplayStart(other.strDisplayStart), maxWidth(other.maxWidth),
    depressedKey(other.depressedKey),
    depressedKeyTimeNext(other.depressedKeyTimeNext),
    insertMode(other.insertMode), destString(other.destString),
    bounds(other.bounds),
    cursorPos(other.cursorPos), textaction(other.textaction)
{
    if(other.excludedCharacters) {
        excludedCharacters = new char[strlen(other.excludedCharacters)+1];
        strcpy(excludedCharacters, other.excludedCharacters);
    } else {
        excludedCharacters = 0;
    }
    inputFieldSurface.copy(other.inputFieldSurface);
}

cInputField::~cInputField()
{
    delete[] excludedCharacters;
}

void cInputField::operator= (const cInputField& other)
{
    pos = other.pos;
    maxCharCount = other.maxCharCount;
    strDisplayStart = other.strDisplayStart;
    maxWidth = other.maxWidth;
    depressedKey = other.depressedKey;
    depressedKeyTimeNext = other.depressedKeyTimeNext;
    insertMode = other.insertMode;
    destString = other.destString;
    if(other.excludedCharacters) {
        excludedCharacters = new char[strlen(other.excludedCharacters) +1];
        strcpy(excludedCharacters, other.excludedCharacters);
    } else {
        excludedCharacters = 0;
    }
    destString = other.destString;
    bounds = other.bounds;
    cursorPos = other.cursorPos;
    textaction = other.textaction;
    inputFieldSurface.copy(other.inputFieldSurface);
}

// reset
//--------------------------------------------------------------------------
void cInputField::reset()
{
    cursorPos          = 0;
    pos                = iXY(0, 0);
    maxCharCount       = 0;
    destString         = 0;
    excludedCharacters = 0;
    textaction         = 0;
    strDisplayStart    = 0;
    depressedKey       = 0;
    depressedKeyTimeNext = 0;
    insertMode         = true;
} // end reset

void cInputField::resetString()
{
    strDisplayStart    = 0;
    cursorPos          = 0;
}

// setPos
//--------------------------------------------------------------------------
void cInputField::setPos(iXY pos)
{
    this->pos  = pos;
    bounds.min = pos;
} // end setPos

// setInputFieldString
// Purpose: Make sure you call setPos before you call this so the
//          bounds get set correctly.
//--------------------------------------------------------------------------
void cInputField::setInputFieldString(cInputFieldString *string)
{
    //assert(string->string != 0);
    assert(string->maxCharCount > 0);

    this->destString   = string->string;
    this->maxCharCount = string->maxCharCount;
    this->maxWidth = string->maxWidth;

    iXY size;
    // XXX (8 is hardcoded here...)
    size.x = string->maxWidth * 8 + 8;
    size.y = Surface::getFontHeight() + 4;

    bounds.max = bounds.min + size;

    inputFieldSurface.create(size.x, size.y, 1);
    inputFieldSurface.fill(Color::black);

} // end setInputFieldString

// setExcludedCharacters
//--------------------------------------------------------------------------
void cInputField::setExcludedCharacters(const char *exChars)
{
    if ( excludedCharacters ) {
        delete excludedCharacters;
        excludedCharacters = 0;
    }
    excludedCharacters = new char [strlen(exChars) + 1];
    if (excludedCharacters == 0) {
        throw Exception("ERROR: Unable to allocate cInputField excludedCharacters.");
    }

    strcpy(excludedCharacters, exChars);
} // setExcludedCharacters

// addChar
//--------------------------------------------------------------------------
void cInputField::addChar(int newChar)
{
    pressKey(newChar);
    // Check if the character should be excluded.
    if (strchr(excludedCharacters, newChar) == 0) {
        
        // Add the character.
        if(insertMode) {
            // ins, move stuff forward
            if((cursorPos+1)<maxCharCount) {
                memmove(destString+cursorPos+1,destString+cursorPos,
                    maxCharCount-(cursorPos+1));
            }
        }
        int replacedChar=destString[cursorPos];
        destString[cursorPos] = newChar;
        if (cursorPos <= maxCharCount) {
            if(replacedChar=='\0') destString[cursorPos+1] = '\0';
        } else {
            destString[maxCharCount] = '\0';
        }

        assert(cursorPos < maxCharCount);

        if (cursorPos < maxCharCount - 1) {
            cursorPos++;
        }
    }

    if(textaction != 0)
        textaction(this);
} // addChar

void cInputField::setTextAction(ACTION_FUNC_PTR action)
{
    textaction = action;
}

// addExtendedChar
//--------------------------------------------------------------------------
void cInputField::addExtendedChar(int newExtendedChar)
{
    pressKey(newExtendedChar);
    // Process the extendedChar accordingly.
    switch (newExtendedChar) {
    case SDLK_HOME: {
            cursorPos = 0;
        }
        break;

    case SDLK_LEFT: {
            if(cursorPos>0)
                cursorPos--;
        }
        break;

    case SDLK_RIGHT: {
            size_t length = strlen(destString);
            ++cursorPos;
            if ( cursorPos >= maxCharCount ) {
                cursorPos = maxCharCount - 1;
            } else if ( cursorPos > length ) {
                cursorPos = length;
            }
        }
        break;

    case SDLK_END: {
            cursorPos = strlen(destString);

            if (cursorPos >= maxCharCount) {
                cursorPos = maxCharCount - 1;
            }
        }
        break;

    case SDLK_INSERT: {
            insertMode^=1;
        }
        break;

    case SDLK_DELETE: {
            if (cursorPos == strlen(destString)) {
                break;
            }

            memcpy(destString + cursorPos, destString + cursorPos + 1, strlen(destString + cursorPos + 1) + 1);
        }
        break;

    case SDLK_BACKSPACE: {
            if (cursorPos >= 1) {
                int byteCount = strlen(destString + cursorPos);

                // Only do this if we are not at the end of the string.
                if (byteCount > 0) {
                    memcpy(destString + cursorPos - 1, destString + cursorPos, byteCount);
                }

                cursorPos--;
                destString[cursorPos + byteCount] = '\0';
            }

        }
        break;

    case SDLK_KP_ENTER:
    case SDLK_RETURN:
        break;

    default: break;
    }

    if(textaction != 0)
        textaction(this);
} // addExtendedChar

// draw
//--------------------------------------------------------------------------
void cInputField::draw(Surface &dest)
{
    checkCursor();
    checkRepeat();

    inputFieldSurface.fill(Color::black);
    inputFieldSurface.drawButtonBorder(Color::white, Color::gray64);
    inputFieldSurface.bltString(4, 2, destString+strDisplayStart, Color::white);
    inputFieldSurface.blt(dest, pos.x, pos.y);
} // draw

// drawHighlighted
//--------------------------------------------------------------------------
void cInputField::drawHighlighted(Surface &dest)
{
    checkCursor();
    checkRepeat();

    inputFieldSurface.fill(Color::black);
    inputFieldSurface.drawButtonBorder(Color::darkGray, Color::white);
    inputFieldSurface.bltStringShadowed(4, 2,
                                        destString+strDisplayStart,
                                        Color::white,
                                        Color::black);

    static float timeForBlink = 0.0f;
    if ((timeForBlink += TimerInterface::getTimeSlice()) > 0.25f) {
        if (timeForBlink > 0.50f) {
            timeForBlink = 0.0f;
        }
    } else {
        int cursorPos=cInputField::cursorPos-strDisplayStart;
        if ((size_t)cursorPos >= maxCharCount) {
            // XXX hardcoded CHAR_PIXX (8)
            inputFieldSurface.bltString(((cursorPos - 1) * 8) + 4, 2, "_", Color::red);
        } else {
            // XXX hardcoded CHAR_PIXX(8)
            inputFieldSurface.bltString(cursorPos * 8 + 4, 2, "_", Color::red);
        }
    }

    inputFieldSurface.blt(dest, pos.x, pos.y);
} // drawHighlighted

// checkCursor
//--------------------------------------------------------------------------
// Purpose: Make sure the cursor stays at the end of the string if the cursor
//          is past the string.
//--------------------------------------------------------------------------
void cInputField::checkCursor()
{
    if (cursorPos > strlen(destString)) {
        cursorPos = strlen(destString);
    }
    if(((size_t)strDisplayStart)>cursorPos) {
        strDisplayStart=cursorPos;
    }
    else if(((size_t)(strDisplayStart+maxWidth))<=cursorPos) {
        strDisplayStart=cursorPos-maxWidth;
    }
} // end checkCursor

void cInputField::pressKey(int ch)
{
    depressedKey=ch;
    depressedKeyTimeNext=SDL_GetTicks()+250;
}

// check repeat and insert characters as needed
void cInputField::checkRepeat()
{
    if(depressedKey==0) { return; }
    Uint32 ticks=SDL_GetTicks();
    if(depressedKeyTimeNext>ticks) {
        return;
    }
    if(KeyboardInterface::getKeyState(depressedKey)!=true) {
        // we've let go of this key.
        depressedKey=0;
        return;
    }

    if(isprint(depressedKey)) {
        addChar(depressedKey);
    }
    else { addExtendedChar(depressedKey); }

    depressedKeyTimeNext=ticks+50;
}

