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

#include <stdint.h>
#include <string>

namespace JOYSTICK
{
  class CJoystickInterfaceLinux;

  class CJoystickLinux : public CJoystick
  {
  public:
    CJoystickLinux(int fd, const std::string& strFilename, CJoystickInterfaceLinux* api);
    virtual ~CJoystickLinux(void) { Deinitialize(); }

    virtual bool Initialize(void) { return true; }
    virtual void Deinitialize(void);

    virtual bool GetEvents(std::vector<ADDON::PeripheralEvent>& events);

  private:
    int         m_fd;
    std::string m_strFilename; // For debugging purposes
  };
}
