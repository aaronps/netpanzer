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
#ifndef __VehicleSelectionView_hpp__
#define __VehicleSelectionView_hpp__

#include "Core/CoreTypes.hpp"
#include "2D/Surface.hpp"
#include "Views/Components/View.hpp"
#include "GameTemplateView.hpp"
#include "Units/UnitProfileInterface.hpp"
#include "Views/Components/Component.hpp"
#include "2D/Components/Button.hpp"


// The following may be used some day.
/*
class NETPANZER_UNIT
{
public:
  Surface hull;
  Surface turret;
  enum { NUM_CHARS = 40 };
 
  char length[NUM_CHARS];
  char width[NUM_CHARS];
  char height[NUM_CHARS];
  char weight[NUM_CHARS];
  char engine[NUM_CHARS];
  char speed[NUM_CHARS];
  char radiusOfAction[NUM_CHARS];
  char groundPressure[NUM_CHARS];
  char armament[NUM_CHARS];
  char crew[NUM_CHARS];
  char notes[NUM_CHARS];
 
}; // end UNIT
*/

extern int vsvSelectedUnit;     // Vehicle Selection View Selected Unit
extern int vsvTempSelectedUnit; // Vehicle Selection View Temp Selected Unit

#define activateVehicleSelectionView(x)
#define toggleDisplayOutpostNames()

//void activateVehicleSelectionView( ObjectiveID outpost_id ){}
//void toggleDisplayOutpostNames( void ){}

//class BoxedLabel;

//--------------------------------------------------------------------------
class VehicleSelectionView : public GameTemplateView
{
private:
    friend class UnitSelectionButton;
    int highlightedUnitType;

    static Button * buttonStaticDisplay;
    static Button * buttonPower;
    static Button * buttonOk;
    
//    BoxedLabel * statusBar;

    int maxHitPoints;
    int maxAttackFactor;
    int maxAttackRange;
    int maxDefendRange;
    int maxTotalSpeed;
    int maxReloadTime;
    int maxRegenTime;
    int unitProfileDataY;

    iXY buttonSize;

    int  getUnitRegenTime(unsigned short unitType);
    void checkMaxValues(const UnitProfile &profile);
    void getProfileData();
    void drawBar(Surface &dest, const iXY &pos, int length, float percent);
    void drawUnitProfileInfo(Surface &dest, const iXY &pos, short int unitType);

    iXY            productionUnitPos;
    iXY            timeRequiredPos;
    iXY            timeRemainingPos;
    iXY            unitsBuiltPos;

    static void checkMiniProductionRect(const std::string& string);

public:
    VehicleSelectionView();
    virtual ~VehicleSelectionView()
    {}

    virtual void        doActivate();
    virtual void        doDraw( Surface& dest );
    virtual void        mouseMove(const iXY &prevPos, const iXY &newPos);
    static  const char *getUnitName(int unitType);
    static  void        drawMiniProductionStatus(Surface &dest);
    static  void        drawUnitImage(Surface &dest, const iXY &pos, int unitType);
    static  bool        displayMiniProductionStatus;
    static  bool        displayOutpostNames;
    static  void        setPowerOn();
    static  void        setPowerOff();
    virtual void        doDeactivate();

    static iRect   miniProductionRect;
}; // end VehicleSelectionView

#endif // end __VehicleSelectionView_hpp__
