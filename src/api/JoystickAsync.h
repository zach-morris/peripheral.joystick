/*
 *      Copyright (C) 2014-2015 Garrett Brown
 *      Copyright (C) 2014-2015 Team XBMC
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

#include "Joystick.h"

#include "threads/mutex.h"

namespace JOYSTICK
{
  /*!
   * Protect several methods of CJoystick with a mutex for thread safety.
   *
   * For joysticks with events that arrive asynchronously.
   */
  class CJoystickAsync : public CJoystick
  {
  public:
    CJoystickAsync(IJoystickInterface* api);
    virtual ~CJoystickAsync(void) { }

    virtual bool GetEvents(std::vector<ADDON::PeripheralEvent>& events);

  protected:
    virtual bool ScanEvents(void) { return true; } // Events arrive asynchronously

    virtual void SetButtonValue(unsigned int buttonIndex, JOYSTICK_STATE_BUTTON buttonValue);
    virtual void SetHatValue(unsigned int hatIndex, JOYSTICK_STATE_HAT hatValue);
    virtual void SetAxisValue(unsigned int axisIndex, JOYSTICK_STATE_AXIS axisValue);

  private:
    PLATFORM::CMutex m_mutex;
  };
}
