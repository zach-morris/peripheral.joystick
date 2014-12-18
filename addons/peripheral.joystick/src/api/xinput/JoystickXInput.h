#pragma once
/*
*      Copyright (C) 2012-2013 Team XBMC
*      http://www.xbmc.org
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

#include <windows.h>

namespace JOYSTICK
{
  class CJoystickXInput : public IJoystick
  {
  public:
    CJoystickXInput(void) { }
    virtual ~CJoystickXInput(void) { }

    virtual bool Initialize(void) { return true;  }
    virtual void Deinitialize(void);

    virtual PERIPHERAL_ERROR PerformJoystickScan(std::vector<ADDON::JoystickConfiguration>& joysticks);

    virtual bool GetEvents(EventMap& events);

    // TODO: IsXInputDevice() from JoystickDirectInput.cpp
    static bool IsXInputDevice(const GUID* pGuidProductFromDirectInput);

  private:
    struct XInputJoystick
    {
      unsigned int m_controllerID;   // XInput port, in the range (0, 3)
      DWORD        m_dwPacketNumber; // If unchanged, controller state hasn't changed (currently ignored)
      ADDON::JoystickConfiguration m_configuration;
    };

    std::vector<XInputJoystick> m_joysticks;
  };
}
