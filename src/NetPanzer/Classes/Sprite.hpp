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
#ifndef _SPRITE_HPP
#define _SPRITE_HPP

#include "2D/Surface.hpp"

#define _MAX_HEIGHT_LEVELS 8

class Sprite
{
public:
    iXY world_pos;
    iXY attach_offset;

    unsigned char height;
    bool       visible;

public:
    Sprite();
	virtual ~Sprite()
	{ }

    inline void setWorldPos(const iXY &worldPos )
    {
        world_pos = worldPos;
    }

    virtual bool isVisible(const iRect &world_win ) const = 0;

    inline void setWorldPos( int x, int y )
    {
        world_pos.x = x;
        world_pos.y = y;
    }

    inline void setAttrib(const iXY &worldPos, const iXY &attach,
                          unsigned char height )
    {
        world_pos = worldPos;
        attach_offset = attach;
        assert( height < _MAX_HEIGHT_LEVELS );
        Sprite::height = height;
    }

    inline void setAttrib(const iXY &worldPos, unsigned char height )
    {
        world_pos = worldPos;
        assert( height < _MAX_HEIGHT_LEVELS );
        Sprite::height = height;
        attach_offset.x = 0;
        attach_offset.y = 0;
    }

    inline void setSpriteHeight( unsigned char height )
    {
        assert( height < _MAX_HEIGHT_LEVELS );
        Sprite::height = height;
    }

    inline unsigned char getSpriteHeight() const
    {
        return( height );
    }

    inline void setVisiblity(bool visible)
    {
        Sprite::visible = visible;
    }

    inline void attachSprite( Sprite *sprite, const iXY &attach )
    {
        Sprite *iterator;

        iterator = this;

        while(iterator->next != 0)
            iterator = iterator->next;

        sprite->setAttrib( world_pos, attach, height );

        iterator->next = sprite;
    }

    virtual void blit( Surface *surface, const iRect &world_win ) = 0;

    inline void blitAll( Surface *surface, const iRect &world_win )
    {
        Sprite *iterator;

        iterator = this;

        while(iterator != 0) {
            iterator->blit( surface, world_win );
            iterator = iterator->next;
        }
    }

protected:
    Sprite *next;

};

class SpriteSurface : public Surface, public Sprite
{
public:

    SpriteSurface();

    inline void setData( Surface &surface )
    {
        setTo(surface);
        setOffsetCenter();
    }

    inline void setAttrib( iXY &worldPos, iXY &attach, unsigned char height, int frame )
    {
        world_pos = worldPos;
        attach_offset = attach;
        assert( height < _MAX_HEIGHT_LEVELS );
        Sprite::height = height;
        setFrame( frame );
    }

    inline void setAttrib( iXY &worldPos, iXY &attach, unsigned char height )
    {
        world_pos = worldPos;
        attach_offset = attach;
        assert( height < _MAX_HEIGHT_LEVELS );
        Sprite::height = height;
    }

    inline void setAttrib( const iXY &worldPos, unsigned char height )
    {
        world_pos = worldPos;
        assert( height < _MAX_HEIGHT_LEVELS );
        Sprite::height = height;
        attach_offset.x = 0;
        attach_offset.y = 0;
    }

    virtual bool isVisible(const iRect &world_win ) const;

    virtual void blit( Surface *surface, const iRect &world_win );
};

class SpritePacked : public Surface, public Sprite
{
private:
//    enum DRAW_MODE
//    {
//        BLEND,
//        SOLID
//    };

//    ColorTable *colorTable;
//    DRAW_MODE   drawMode;

public:

    SpritePacked();

    SpritePacked( Surface &source );

    virtual ~SpritePacked()
    { }

// XXX NOT NOW MAYBE DELETE
//    inline void setDrawModeBlend(ColorTable *colorTable)
//    {
//        (void)colorTable;
//        SpritePacked::colorTable = colorTable;
//        SpritePacked::drawMode   = BLEND;
//    }

//    inline void setDrawModeSolid()
//    {
//        drawMode = SOLID;
//    }


    inline void setData( Surface &source )
    {
        setTo( source );
        setOffsetCenter();
    }

    inline void setAttrib( const iXY &worldPos, const iXY &attach,
                           unsigned char height, int frame )
    {
        world_pos = worldPos;
        attach_offset = attach;
        assert( height < _MAX_HEIGHT_LEVELS );
        Sprite::height = height;
        setFrame( frame );
    }


    inline void setAttrib( const iXY &worldPos, const iXY &attach,
                           unsigned char height )
    {
        world_pos = worldPos;
        attach_offset = attach;
        assert( height < _MAX_HEIGHT_LEVELS );
        Sprite::height = height;
    }

    inline void setAttrib( const iXY &worldPos, unsigned char height )
    {
        world_pos = worldPos;
        assert( height < _MAX_HEIGHT_LEVELS );
        Sprite::height = height;
        attach_offset.x = 0;
        attach_offset.y = 0;
    }

    virtual bool isVisible(const iRect &world_win ) const;

    virtual void blit( Surface *surface, const iRect &world_win );
};

#endif // ** _SPRITE_HPP
