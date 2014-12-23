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
#include "XInputDLL.h"
#include "log/Log.h"

#include <Xinput.h>

using namespace JOYSTICK;

#define XINPUT_ALIAS  "Xbox 360-compatible controller"
#define BUTTON_COUNT  15
#define HAT_COUNT     0 // hats are treated as buttons
#define AXIS_COUNT    6
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

bool CJoystickXInput::Initialize(void)
{
  m_stateBuffer.buttons.assign(ButtonCount(), JOYSTICK_STATE_BUTTON());
  m_stateBuffer.axes.assign(ButtonCount(), JOYSTICK_STATE_ANALOG());

  return CJoystick::Initialize();
}

bool CJoystickXInput::GetEvents(std::vector<ADDON::PeripheralEvent>& events)
{
  XINPUT_STATE_EX controllerState;

  if (!CXInputDLL::Get().GetState(m_controllerID, controllerState))
    return false;

  m_dwPacketNumber = controllerState.dwPacketNumber;

  std::vector<JOYSTICK_STATE_BUTTON>& buttons = m_stateBuffer.buttons;
  buttons[0]  = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_A)              ? JOYSTICK_STATE_BUTTON_PRESSED : JOYSTICK_STATE_BUTTON_UNPRESSED;
  buttons[1]  = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_B)              ? JOYSTICK_STATE_BUTTON_PRESSED : JOYSTICK_STATE_BUTTON_UNPRESSED;
  buttons[2]  = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_X)              ? JOYSTICK_STATE_BUTTON_PRESSED : JOYSTICK_STATE_BUTTON_UNPRESSED;
  buttons[3]  = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_Y)              ? JOYSTICK_STATE_BUTTON_PRESSED : JOYSTICK_STATE_BUTTON_UNPRESSED;
  buttons[4]  = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)  ? JOYSTICK_STATE_BUTTON_PRESSED : JOYSTICK_STATE_BUTTON_UNPRESSED;
  buttons[5]  = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) ? JOYSTICK_STATE_BUTTON_PRESSED : JOYSTICK_STATE_BUTTON_UNPRESSED;
  buttons[6]  = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK)           ? JOYSTICK_STATE_BUTTON_PRESSED : JOYSTICK_STATE_BUTTON_UNPRESSED;
  buttons[7]  = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_START)          ? JOYSTICK_STATE_BUTTON_PRESSED : JOYSTICK_STATE_BUTTON_UNPRESSED;
  buttons[8]  = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)     ? JOYSTICK_STATE_BUTTON_PRESSED : JOYSTICK_STATE_BUTTON_UNPRESSED;
  buttons[9]  = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)    ? JOYSTICK_STATE_BUTTON_PRESSED : JOYSTICK_STATE_BUTTON_UNPRESSED;
  buttons[10] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP)        ? JOYSTICK_STATE_BUTTON_PRESSED : JOYSTICK_STATE_BUTTON_UNPRESSED;
  buttons[11] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)     ? JOYSTICK_STATE_BUTTON_PRESSED : JOYSTICK_STATE_BUTTON_UNPRESSED;
  buttons[12] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)      ? JOYSTICK_STATE_BUTTON_PRESSED : JOYSTICK_STATE_BUTTON_UNPRESSED;
  buttons[13] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)      ? JOYSTICK_STATE_BUTTON_PRESSED : JOYSTICK_STATE_BUTTON_UNPRESSED;
  buttons[14] = (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_GUIDE)          ? JOYSTICK_STATE_BUTTON_PRESSED : JOYSTICK_STATE_BUTTON_UNPRESSED;

  // TODO: dyload XInput lib to access guide button through hidden API
  GetButtonEvents(buttons, events);

  std::vector<JOYSTICK_STATE_ANALOG>& axes = m_stateBuffer.axes;
  axes[0] = NormalizeAxis(controllerState.Gamepad.sThumbLX, MAX_AXIS);
  axes[1] = NormalizeAxis(controllerState.Gamepad.sThumbLY, MAX_AXIS);
  axes[2] = NormalizeAxis(controllerState.Gamepad.sThumbRX, MAX_AXIS);
  axes[3] = NormalizeAxis(controllerState.Gamepad.sThumbRY, MAX_AXIS);
  axes[4] = NormalizeAxis(controllerState.Gamepad.bLeftTrigger, MAX_TRIGGER);
  axes[5] = NormalizeAxis(controllerState.Gamepad.bRightTrigger, MAX_TRIGGER);
  GetAxisEvents(axes, events);

  return true;
}
