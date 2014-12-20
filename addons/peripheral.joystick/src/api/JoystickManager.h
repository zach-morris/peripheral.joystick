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

#include "platform/threads/mutex.h"
#include "xbmc_peripheral_utils.hpp"

#include <map>
#include <vector>

namespace JOYSTICK
{
  class CJoystick;
  class CJoystickInterface;

  class CJoystickManager
  {
  private:
    CJoystickManager(void) { }

  public:
    static CJoystickManager& Get(void);
    virtual ~CJoystickManager(void) { Deinitialize(); }

    bool Initialize(void);
    void Deinitialize(void);

    bool PerformJoystickScan(std::vector<CJoystick*>& joysticks);

    CJoystick* GetJoystick(unsigned int index) const;

    /*!
    * @brief Get all events that have occurred since the last call to GetEvents()
    */
    bool GetEvents(std::vector<ADDON::PeripheralEvent>& events);

  private:
    typedef std::map<CJoystickInterface*, std::vector<CJoystick*> > JoystickMap; // TODO

    std::vector<CJoystickInterface*>   m_interfaces;
    JoystickMap                        m_joysticks;
    mutable PLATFORM::CMutex           m_joystickMutex;
  };
}
