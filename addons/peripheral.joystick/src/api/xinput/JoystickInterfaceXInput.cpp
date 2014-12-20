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

#include "JoystickInterfaceXInput.h"
#include "JoystickXInput.h"
#include "api/JoystickTypes.h"
#include "log/Log.h"

#include <Xinput.h>

#pragma comment(lib, "XInput.lib")

using namespace JOYSTICK;

#define MAX_JOYSTICKS 4

CJoystickInterfaceXInput::CJoystickInterfaceXInput(void)
 : CJoystickInterface(INTERFACE_XINPUT)
{
}

bool CJoystickInterfaceXInput::PerformJoystickScan(std::vector<CJoystick*>& joysticks)
{
  Deinitialize();

  XINPUT_STATE controllerState; // No need to memset, only checking for controller existence

  for (unsigned int i = 0; i < MAX_JOYSTICKS; i++)
  {
    DWORD result = XInputGetState(i, &controllerState);
    if (result != ERROR_SUCCESS)
    {
      if (result == ERROR_DEVICE_NOT_CONNECTED)
        dsyslog("No XInput devices on port %u", i);
      continue;
    }

    // That's all it takes to check controller existence... I <3 XInput
    isyslog("Found a XInput controller on port %u", i);
    joysticks.push_back(new CJoystickXInput(i, this));
  }

  return true;
}
