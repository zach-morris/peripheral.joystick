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

#include <string>

struct _SDL_Joystick;
typedef struct _SDL_Joystick SDL_Joystick;

namespace JOYSTICK
{
  class CJoystickSDL : public IJoystick
  {
  public:
    CJoystickSDL(void) { }
    virtual ~CJoystickSDL(void) { Deinitialize(); }

    virtual bool Initialize(void) { return true; }
    virtual void Deinitialize(void);

    virtual PERIPHERAL_ERROR PerformJoystickScan(std::vector<ADDON::JoystickConfiguration>& joysticks);

    virtual bool GetEvents(EventMap& events);

  private:

    struct SDLJoystick
    {
      SDL_Joystick* m_pJoystick;
      ADDON::JoystickConfiguration m_configuration;
    };

    std::vector<SDLJoystick> m_joysticks;
  };
}
