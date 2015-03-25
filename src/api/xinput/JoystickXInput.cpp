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
  SetRequestedPort(m_controllerID);
  SetButtonCount(BUTTON_COUNT);
  SetHatCount(HAT_COUNT);
  SetAxisCount(AXIS_COUNT);

  Features().push_back(new ADDON::DriverButton(JOYSTICK_FEATURE_BUTTON_A,       0));
  Features().push_back(new ADDON::DriverButton(JOYSTICK_FEATURE_BUTTON_B,       1));
  Features().push_back(new ADDON::DriverButton(JOYSTICK_FEATURE_BUTTON_X,       2));
  Features().push_back(new ADDON::DriverButton(JOYSTICK_FEATURE_BUTTON_Y,       3));
  Features().push_back(new ADDON::DriverButton(JOYSTICK_FEATURE_BUTTON_L,       4));
  Features().push_back(new ADDON::DriverButton(JOYSTICK_FEATURE_BUTTON_R,       5));
  Features().push_back(new ADDON::DriverButton(JOYSTICK_FEATURE_BUTTON_SELECT,  6));
  Features().push_back(new ADDON::DriverButton(JOYSTICK_FEATURE_BUTTON_START,   7));
  Features().push_back(new ADDON::DriverButton(JOYSTICK_FEATURE_BUTTON_L_STICK, 8));
  Features().push_back(new ADDON::DriverButton(JOYSTICK_FEATURE_BUTTON_R_STICK, 9));
  Features().push_back(new ADDON::DriverButton(JOYSTICK_FEATURE_BUTTON_UP,      10));
  Features().push_back(new ADDON::DriverButton(JOYSTICK_FEATURE_BUTTON_RIGHT,   11));
  Features().push_back(new ADDON::DriverButton(JOYSTICK_FEATURE_BUTTON_DOWN,    12));
  Features().push_back(new ADDON::DriverButton(JOYSTICK_FEATURE_BUTTON_LEFT,    13));
  Features().push_back(new ADDON::DriverButton(JOYSTICK_FEATURE_BUTTON_HOME,    14));

  Features().push_back(new ADDON::DriverAnalogStick(JOYSTICK_FEATURE_ANALOG_STICK_L, 0, false, 1, false));
  Features().push_back(new ADDON::DriverAnalogStick(JOYSTICK_FEATURE_ANALOG_STICK_R, 2, false, 3, false));

  Features().push_back(new ADDON::DriverSemiAxis(JOYSTICK_FEATURE_TRIGGER_L, 4, JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE));
  Features().push_back(new ADDON::DriverSemiAxis(JOYSTICK_FEATURE_TRIGGER_R, 5, JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE));
}

bool CJoystickXInput::Initialize(void)
{
  m_stateBuffer.buttons.assign(ButtonCount(), JOYSTICK_STATE_BUTTON());
  m_stateBuffer.axes.assign(AxisCount(), JOYSTICK_STATE_AXIS());

  return CJoystick::Initialize();
}

bool CJoystickXInput::ScanEvents(std::vector<ADDON::PeripheralEvent>& events)
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

  std::vector<JOYSTICK_STATE_AXIS>& axes = m_stateBuffer.axes;
  axes[0] = NormalizeAxis(controllerState.Gamepad.sThumbLX, MAX_AXIS);
  axes[1] = NormalizeAxis(controllerState.Gamepad.sThumbLY, MAX_AXIS);
  axes[2] = NormalizeAxis(controllerState.Gamepad.sThumbRX, MAX_AXIS);
  axes[3] = NormalizeAxis(controllerState.Gamepad.sThumbRY, MAX_AXIS);
  axes[4] = NormalizeAxis(controllerState.Gamepad.bLeftTrigger, MAX_TRIGGER);
  axes[5] = NormalizeAxis(controllerState.Gamepad.bRightTrigger, MAX_TRIGGER);
  GetAxisEvents(axes, events);

  return true;
}
