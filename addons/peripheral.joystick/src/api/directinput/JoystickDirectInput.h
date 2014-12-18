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

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <string>

namespace JOYSTICK
{
  class CJoystickDirectInput : public IJoystick
  {
  public:
    CJoystickDirectInput(void);
    virtual ~CJoystickDirectInput(void) { Deinitialize(); }

    virtual bool Initialize(void) { return true; }
    virtual void Deinitialize(void);

    virtual PERIPHERAL_ERROR PerformJoystickScan(std::vector<ADDON::JoystickConfiguration>& joysticks);

    virtual bool GetEvents(EventMap& events);

  private:
    static BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE *pdidInstance, VOID *pContext);
    static BOOL CALLBACK EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE *pdidoi, VOID *pContext);
    static bool IsXInputDevice(const GUID *pGuidProductFromDirectInput);

    struct DirectInputJoystick
    {
      LPDIRECTINPUTDEVICE8         m_joystickDevice;
      ADDON::JoystickConfiguration m_configuration;
    };

    std::vector<DirectInputJoystick> m_joysticks;
    LPDIRECTINPUT8                   m_pDirectInput; // DirectInput handle, we hold onto it and release it when freeing resources
  };
}
