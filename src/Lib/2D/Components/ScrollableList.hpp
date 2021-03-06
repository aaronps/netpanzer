/*
Copyright (C) 2013 Netpanzer Team. (www.netpanzer.org), Aaron Perez

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
 *
 * Created on August 29, 2013, 10:18 PM
 */

#ifndef SCROLLABLELIST_HPP
#define	SCROLLABLELIST_HPP

#include "Views/Components/Component.hpp"
#include "Types/UString.hpp"
#include "ArrayUtil/PtrArray.hpp"
#include "2D/Color.hpp"
#include <list>

class VScrollBar;
class TextRenderer;

class ScrollableList : public Component
{
public:
    ScrollableList(const int width, const int height);
    ~ScrollableList();
    
    /**
     * Sets the maximum number of lines that the ScrollableList may contain.
     * @param n The maximum number of lines
     */
    void setMaxLines(const unsigned n);

    /**
     * Adds a line of text to the list
     * @param str The text
     * @param firstLineOffset The x offset of the first character, defaults to 0
     * @param color The drawing color of the text
     */
    void addLine(const UString& str, const int firstLineOffset = 0, const uint8_t color = Color::gray224);
    
    void setLocation(const int x, const int y);
    void setSize(const int x, const int y);
    
private:
    void draw(Surface& s) const;
    void logic();
    
    void onSelectStart();
    void onSelectStop();

    void onHoverStart();
    void onHoverStop();
    
    void onPointerMove(int rel_x, int rel_y);
    
    /** Holds one element of the list, a line of text and its related data */
    struct ListElement
    {
        ListElement( const UString& str, const int height, const int firstLineOffset, uint8_t color)
            : str(str), height(height), firstLineOffset(firstLineOffset), color(color) {}
        
        UString str;
        int     height;
        int     firstLineOffset;
        uint8_t   color;
    };
    
    /** lines list */
    std::list<ListElement> lines;
    
    /** first visible line iterator, used for scrolling */
    std::list<ListElement>::iterator firstVisibleLineIterator;
    
    /** sum of all the rendered lines heigh, one line might be broken in several */
    int totalLinesHeight;
    
    /** Offset of the first TextRenderer */
    int rendedTotalDisplayStart; // @todo find a better name for this variable
    
    /** max number of lines in the list */
    unsigned maxLines;
    
    /** Width of the visible are for the text lines of the ScrollableList */    
    int visibleWidth;
    
    /** Offset (y) to begin drawing the first renderer */
    int drawRenderOffset;
    
    /** array of visible TextRenderers, used in draw() */
    PtrArray<TextRenderer> visibleRenders;
    
    /** shall redraw the text lines? */
    bool dirty;

    /** The scroll bar */
    VScrollBar * scrollbar;
    
    /** flag if the mouse is over the scrollbar */
    bool hoveringScrollbar;
    
    /** flag if the mouse is pressing the scrollbar */
    bool selectingScrollbar;
    
    /** Scrollbar will send the change event here */
    ComponentEvents myEvents;
};

#endif	/* SCROLLABLELIST_HPP */

