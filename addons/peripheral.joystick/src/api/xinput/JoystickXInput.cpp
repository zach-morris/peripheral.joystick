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

#include "JoystickXInput.h"
#include "log/Log.h"

#include <Xinput.h>

#pragma comment(lib, "XInput.lib")

using namespace ADDON;
using namespace JOYSTICK;

#define XINPUT_ALIAS  "XBMC-Compatible XInput Controller"
#define MAX_JOYSTICKS 4
#define MAX_AXIS      32768
#define MAX_TRIGGER   255
#define BUTTON_COUNT  10
#define HAT_COUNT     1
#define AXIS_COUNT    5

void CJoystickXInput::Deinitialize(void)
{
  m_joysticks.clear();
}

PERIPHERAL_ERROR CJoystickXInput::PerformJoystickScan(std::vector<JoystickConfiguration>& joysticks)
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

    XInputJoystick joystick;
    joystick.m_controllerID = i;
    joystick.m_dwPacketNumber = 0;
    joystick.m_configuration.SetIndex(0); // Set by CJoystickManager
    joystick.m_configuration.SetRequestedPlayer(i);
    joystick.m_configuration.SetName(XINPUT_ALIAS);
    joystick.m_configuration.SetIconPath(""); // TODO

    for (unsigned int i = 0; i < BUTTON_COUNT; i++)
      joystick.m_configuration.ButtonIndexes().push_back(i);
    for (unsigned int i = 0; i < HAT_COUNT; i++)
      joystick.m_configuration.HatIndexes().push_back(i);
    for (unsigned int i = 0; i < AXIS_COUNT; i++)
      joystick.m_configuration.AxisIndexes().push_back(i);

    m_joysticks.push_back(joystick);

    joysticks.push_back(joystick.m_configuration);
  }

  return PERIPHERAL_NO_ERROR;
}

bool CJoystickXInput::GetEvents(EventMap& events)
{
#if 0
  for (std::vector<XInputJoystick>::iterator it = m_joysticks.begin(); it != m_joysticks.end(); ++it)
  {
    CJoystickState &state = InitialState();

    XINPUT_STATE controllerState;

    DWORD result = XInputGetState(it->m_controllerID, &controllerState);
    if (result != ERROR_SUCCESS)
      return;

    it->m_dwPacketNumber = controllerState.dwPacketNumber;

    // Map to DirectInput controls
    state.buttons[0] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_A) ? true : false;
    state.buttons[1] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_B) ? true : false;
    state.buttons[2] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_X) ? true : false;
    state.buttons[3] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_Y) ? true : false;
    state.buttons[4] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) ? true : false;
    state.buttons[5] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) ? true : false;
    state.buttons[6] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) ? true : false;
    state.buttons[7] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_START) ? true : false;
    state.buttons[8] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) ? true : false;
    state.buttons[9] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) ? true : false;

    state.hats[0][CJoystickHat::UP] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) ? true : false;
    state.hats[0][CJoystickHat::RIGHT] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) ? true : false;
    state.hats[0][CJoystickHat::DOWN] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) ? true : false;
    state.hats[0][CJoystickHat::LEFT] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) ? true : false;

    // Combine triggers into a single axis, like DirectInput
    const long triggerAxis = (long)controllerState.Gamepad.bLeftTrigger - (long)controllerState.Gamepad.bRightTrigger;
    state.SetAxis(0, controllerState.Gamepad.sThumbLX, MAX_AXIS);
    state.SetAxis(1, -controllerState.Gamepad.sThumbLY, MAX_AXIS);
    state.SetAxis(2, triggerAxis, MAX_TRIGGER);
    state.SetAxis(3, controllerState.Gamepad.sThumbRX, MAX_AXIS);
    state.SetAxis(4, -controllerState.Gamepad.sThumbRY, MAX_AXIS);

    UpdateState(state);
  }
#endif
  return false;
}
