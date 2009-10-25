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


#include "CloudParticle2D.hpp"
#include "Util/TimerInterface.hpp"
#include "Interfaces/GameConfig.hpp"
#include "Util/Exception.hpp"
#include "2D/Palette.hpp"
#include "Classes/Sprite.hpp"


// Statics.
Surface CloudParticle2D::staticPackedCloud;
Surface CloudParticle2D::staticCloudShadow;
iXY CloudParticle2D::worldSize(0, 0);


// CloudParticle2D
//---------------------------------------------------------------------------
CloudParticle2D::CloudParticle2D(	const fXYZ &pos,
                                  const iXY  &worldSize,
                                  float       windMin,
                                  float       windRand) : WindParticle2D(pos)
{
    // How much does the ambient wind affect this cloud.
    windScale = getScale(windMin, windRand);

    CloudParticle2D::worldSize = worldSize;

    // Randomly select a cloud and a blend mode.
    setRandomSurface();

    // Make the particle live forever.
    lifetime  = -1;

} // end CloudParticle2D::CloudParticle2D

// setRandomSurface
//---------------------------------------------------------------------------
// Purpose: Sets the cloud image to a random cloud and sets the blending
//          level to a random table.
//---------------------------------------------------------------------------
void CloudParticle2D::setRandomSurface()
{
    packedSurface.setData(staticPackedCloud);

    int randFrame = rand() % staticPackedCloud.getNumFrames();
    packedSurface.setFrame(randFrame);

    packedSurfaceShadow.setData(staticCloudShadow);
    packedSurfaceShadow.setFrame(randFrame);

    packedSurfaceShadow.setDrawModeBlend(32);

    packedSurface.setDrawModeBlend(128);

} // end CloudParticle2D::setRandomSurface

//---------------------------------------------------------------------------
void CloudParticle2D::packFiles()
{}

//---------------------------------------------------------------------------
void CloudParticle2D::loadPAKFiles()
{
    staticPackedCloud.loadPNGSheet("pics/particles/clouds/clouds.png", 300, 300, 10);
    staticPackedCloud.setColorkey();
//    staticPackedCloud.setOffsetCenter();

    staticCloudShadow.createShadow(staticPackedCloud);
//    staticCloudShadow.setOffsetCenter();
}

// init
//---------------------------------------------------------------------------
void CloudParticle2D::init()
{
    loadPAKFiles();
} // end CloudParticle2D::init

// sim
//---------------------------------------------------------------------------
void CloudParticle2D::sim()
{
    static const bool worldWraps = false;

    // NOTE - Rand will not randomly place the clouds over the whole map,
    //        when the maps get larger than RAND_MAX in pixel dimension.
    // Handle the wrap around.
    if (pos.x + packedSurface.getWidth() < 0) {
        pos.x = worldSize.x + packedSurface.getCenterX();

        if (!worldWraps) {
            pos.z = rand() % (worldSize.y + packedSurface.getCenterY());
            setRandomSurface();
        }
    } else if (pos.x > worldSize.x + packedSurface.getCenterX()) {
        pos.x = packedSurface.getOffsetX();

        if (!worldWraps) {
            setRandomSurface();
        }
    }

    if (pos.z + packedSurface.getHeight() < 0) {
        pos.z = worldSize.y + packedSurface.getCenterY();

        if (!worldWraps) {
            pos.x = rand() % (worldSize.x + packedSurface.getCenterX());
            setRandomSurface();
        }
    } else if (pos.z > worldSize.y + packedSurface.getCenterY()) {
        pos.z = packedSurface.getOffsetY();

        if (!worldWraps) {
            setRandomSurface();
        }
    }

    WindParticle2D::sim();

} // end CloudParticle2D::sim

// draw
//---------------------------------------------------------------------------
void CloudParticle2D::draw(SpriteSorter &sorter)
{
    packedSurface.setAttrib(iXY(int(pos.x), int(pos.z)), layer);
    sorter.addSprite(&packedSurface);

    if (gameconfig->displayshadows) {
        packedSurfaceShadow.setAttrib(iXY(int(pos.x - 300), int(pos.z)), shadowLayer);
        sorter.addSprite(&packedSurfaceShadow);
    }

} // end CloudParticle2D::draw
