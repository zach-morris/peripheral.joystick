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

#include <vector>

namespace JOYSTICK
{
  class CJoystickInterface;

  class CJoystick : public ADDON::Joystick
  {
  public:
    CJoystick(CJoystickInterface* api);
    virtual ~CJoystick(void) { }

    bool operator==(const CJoystick& rhs) const; // TODO
    bool operator!=(const CJoystick& rhs) const { return !operator==(rhs); }

    virtual bool Initialize(void) = 0;
    virtual void Deinitialize(void) = 0;

    virtual bool GetEvents(std::vector<ADDON::PeripheralEvent>& events) = 0;

  protected:
    CJoystickInterface* m_api;
  };
}
