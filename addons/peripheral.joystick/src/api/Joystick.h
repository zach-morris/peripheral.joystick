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
#include <memory>
#include <vector>

namespace JOYSTICK
{
  typedef std::shared_ptr<ADDON::JoystickEvent> EventPtr;
  typedef std::vector<EventPtr>                 EventVector;
  typedef std::map<unsigned int, EventVector>   EventMap; // Joystick ID -> events

  class IJoystick
  {
  public:
    virtual ~IJoystick(void) { }

    virtual bool Initialize(void) = 0;
    virtual void Deinitialize(void) = 0;

    virtual PERIPHERAL_ERROR PerformJoystickScan(std::vector<ADDON::JoystickConfiguration>& joysticks) = 0;

    virtual bool GetEvents(EventMap& events) = 0;
  };
}
