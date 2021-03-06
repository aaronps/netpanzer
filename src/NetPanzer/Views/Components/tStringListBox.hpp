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
#ifndef __tStringListBox_hpp__
#define __tStringListBox_hpp__

//#include <vector>
#include <list>
#include <string>
#include "2D/Surface.hpp"
#include "Component.hpp"

class DataItem
{
public:
    std::string text;
    int num_lines;
    void * Data;
};

class tStringListBox : public Component
{
protected:
    std::list<DataItem> List;
    void setHasHeader(bool b) { hasHeader = b; dirty = true; }
    bool    dirty;

private:
    enum { ItemHeight = 14 };
    
    Surface surface;
    
    std::list<DataItem>::iterator StartItem;
    int StartSubLine;
    int StartWidth;
    
    std::list<DataItem>::iterator SelectedItem;
    int MaxItemWidth;
    
    int TotalLines;
    int TotalPosition;
    
    bool Selectable;
    bool AutoScroll;
    bool AutoWrap;
    bool Bordered;
    bool hasHeader;
    
    PIX background_color;
    
    void draw(Surface& dest) const;
    virtual void render();
public:

    tStringListBox(iRect rect);
    virtual ~tStringListBox() {}

    void setSelectable( bool b ) { Selectable = b; SelectedItem = List.end(); dirty = true;}
    void setBordered( bool b ) { Bordered = b; dirty = true; }
    
    virtual void setColor(PIX newColor);
    
    virtual int getMaxItemWidth(const DataItem& data);
    virtual std::string getTextItem();
    
    int getNumVisibleLines() { return (getHeight()-5)/ItemHeight; }
    
    virtual void AddBlock(const std::string& S);
    virtual void Add(const std::string& S) { AddData(S, 0); }
    virtual void AddData(const std::string& S, void * D);
    virtual void deleteData(const DataItem& data) { /* to be overriden */}
    
    virtual void Clear();
    
    virtual int Count(){return List.size();}
    virtual void onPaint(Surface &dst, const DataItem& data, int SubLine);
    virtual void paintHeader(Surface &dst) {}
    virtual void setLocation(int x, int y);
    virtual void setLocation(const iXY &p) { setLocation(p.x, p.y); }
    virtual void setAutoScroll(bool Value){AutoScroll = Value;}
    
    void setSize(const iXY& siz)
    {
        setSize(siz.x, siz.y);
    }
    
    void setSize(const int x, const int y)
    {
        Component::setSize(x, y);
        surface.create(x,y);
        dirty=true;
    }
    
    void setAutoWrap(bool autowrap);
    
    virtual int getNumLines( int width, const DataItem& data) { return 1; }
}; 

#endif 
