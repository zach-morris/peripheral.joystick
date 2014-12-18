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

#include "Joystick.h"

#include "platform/threads/mutex.h"
#include "xbmc_peripheral_types.h"

namespace JOYSTICK
{
  class CJoystickManager
  {
  private:
    CJoystickManager(void) { }
    CJoystickManager(const CJoystickManager& other);

  public:
    static CJoystickManager& Get(void);
    virtual ~CJoystickManager(void) { Deinitialize(); }

    bool Initialize(void);
    void Deinitialize(void);

    PERIPHERAL_ERROR PerformJoystickScan(std::vector<ADDON::JoystickConfiguration>& joysticks);

    bool GetEvents(EventMap& events);

  private:
    std::vector<IJoystick*> m_joysticks;
    PLATFORM::CMutex        m_joystickMutex;
  };
}
