/*
 *      Copyright (C) 2014 Garrett Brown
 *      Copyright (C) 2014 Team XBMC
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#pragma once

#include "api/Joystick.h"

#include <SDL/SDL.h>
#include <SDL/SDL_joystick.h>

namespace JOYSTICK
{
  class CJoystickInterfaceSDL;

  class CJoystickSDL : public CJoystick
  {
  public:
    CJoystickSDL(SDL_Joystick* pJoystick, CJoystickInterfaceSDL* api);
    virtual ~CJoystickSDL(void) { Deinitialize(); }

    bool Initialize(void);

  protected:
    virtual bool ScanEvents(std::vector<ADDON::PeripheralEvent>& events);

  private:
    SDL_Joystick* m_pJoystick;
  };
}
