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
#include "JoystickInterfaceXInput.h"
#include "log/Log.h"

#include <Xinput.h>

using namespace JOYSTICK;

#define XINPUT_ALIAS  "Xbox 360-compatible controller"
#define BUTTON_COUNT  10
#define HAT_COUNT     1
#define AXIS_COUNT    5
#define MAX_AXIS      32768
#define MAX_TRIGGER   255

CJoystickXInput::CJoystickXInput(unsigned int controllerID, CJoystickInterfaceXInput* api)
 : CJoystick(api),
   m_controllerID(controllerID),
   m_dwPacketNumber(0)
{
  SetName(XINPUT_ALIAS);
  SetRequestedPlayer(m_controllerID + 1);
  SetButtonCount(BUTTON_COUNT);
  SetHatCount(HAT_COUNT);
  SetAxisCount(AXIS_COUNT);
}

bool CJoystickXInput::GetEvents(std::vector<ADDON::PeripheralEvent>& events)
{
  XINPUT_STATE controllerState;

  DWORD result = XInputGetState(m_controllerID, &controllerState);
  if (result != ERROR_SUCCESS)
    return false;

  m_dwPacketNumber = controllerState.dwPacketNumber;

  /* TODO
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
  */

  return false;
}
