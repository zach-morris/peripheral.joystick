/*
 *      Copyright (C) 2016 Garrett Brown
 *      Copyright (C) 2016 Team Kodi
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
 *  along with this Program; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#pragma once

/*
 * Derived from udev_joypad.c in the RetroArch project.
 */

/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2015 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2016 - Daniel De Matteis
 *
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "api/Joystick.h"

#include <linux/input.h>
#include <sys/types.h>

struct udev_device;

namespace JOYSTICK
{
  class CJoystickUdev : public CJoystick
  {
  public:
    enum
    {
      MOTOR_STRONG = 0,
      MOTOR_WEAK   = 1,
      MOTOR_COUNT  = 2,
    };

    CJoystickUdev(udev_device* dev, const char* path);
    virtual ~CJoystickUdev(void) { Deinitialize(); }

    // implementation of CJoystick
    virtual bool Equals(const CJoystick* rhs) const override;
    virtual bool Initialize(void) override;
    virtual void Deinitialize(void) override;

  protected:
    // implementation of CJoystick
    virtual bool ScanEvents(void) override;
    bool SetMotor(unsigned int motorIndex, float magnitude);

  private:
    struct Motor
    {
      uint16_t strength;
      uint16_t configured_strength;
    };

    struct Axis
    {
      unsigned int  axisIndex;
      input_absinfo axisInfo;
    };

    bool OpenJoystick();
    bool GetProperties();

    // Udev properties
    udev_device* m_dev;
    std::string  m_path;
    dev_t        m_deviceNumber;
    int          m_fd;
    bool         m_bInitialized;
    bool         m_has_set_ff;
    int          m_effect;

    // Joystick properties
    std::map<unsigned int, unsigned int> m_button_bind; // Maps keycodes -> button
    std::map<unsigned int, Axis>         m_axes_bind;   // Maps keycodes -> axis and axis info
    Motor                                m_motors[MOTOR_COUNT];
  };
}
