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
 */
#pragma once

#include "xbmc_peripheral_utils.hpp"

#include <map>
#include <vector>

namespace JOYSTICK
{
  class CJoystickInterface;

  class CJoystick : public ADDON::Joystick
  {
  public:
    CJoystick(CJoystickInterface* api);
    virtual ~CJoystick(void) { }

    virtual bool Initialize(void);
    virtual void Deinitialize(void) { }

    virtual bool GetEvents(std::vector<ADDON::PeripheralEvent>& events) = 0;

    const CJoystickInterface* API(void) const { return m_api; }

  protected:
    void GetButtonEvents(const std::vector<JOYSTICK_STATE_BUTTON>& buttons, std::vector<ADDON::PeripheralEvent>& events);
    void GetHatEvents(const std::vector<JOYSTICK_STATE_HAT>& hats, std::vector<ADDON::PeripheralEvent>& events);
    void GetAxisEvents(const std::vector<JOYSTICK_STATE_AXIS>& axes, std::vector<ADDON::PeripheralEvent>& events);

    static JOYSTICK_STATE_AXIS NormalizeAxis(long value, long maxAxisAmount);

    CJoystickInterface* const m_api;

    struct JoystickState
    {
      std::vector<JOYSTICK_STATE_BUTTON> buttons;
      std::vector<JOYSTICK_STATE_HAT>    hats;
      std::vector<JOYSTICK_STATE_AXIS>   axes;
    };

    JoystickState m_stateBuffer;

  private:
    JoystickState m_state;
  };
}
