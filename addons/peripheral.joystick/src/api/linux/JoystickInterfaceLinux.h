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

#include "api/JoystickInterface.h"

#include <stdint.h>
#include <string>

namespace JOYSTICK
{
  class CJoystickInterfaceLinux : public CJoystickInterface
  {
  public:
    CJoystickInterfaceLinux(void);
    virtual ~CJoystickInterfaceLinux(void) { Deinitialize(); }

    virtual bool Initialize(void) { return true; }
    virtual void Deinitialize(void) { }

  protected:
    virtual bool PerformJoystickScan(std::vector<CJoystick*>& joysticks);

  private:
    /**
     * Retrieves the current button map in the given array, which must contain at
     * least BTNMAP_SIZE elements. Returns the result of the ioctl(): negative in
     * case of an error, 0 otherwise for kernels up to 2.6.30, the length of the
     * array actually copied for later kernels.
     */
    static bool GetButtonMap(int fd, uint16_t *buttonMap);

    /**
     * Retrieves the current axis map in the given array, which must contain at
     * least AXMAP_SIZE elements.
     */
    static bool GetAxisMap(int fd, uint8_t *axisMap);

    /**
     * Try a series of ioctls until one succeeds.
     * @param fd - The fd to perform ioctl on
     * @param ioctls - A zero-terminated list of ioctls
     * @param buttonMap - The discovered button map
     * @param ioctl_used - The ioctl that succeeded (untouched if false is returned)
     */
    static bool DetermineIoctl(int fd, const unsigned long *ioctls, uint16_t *buttonMap, unsigned long &ioctl_used);
  };
}
