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

#include "JoystickLinux.h"
#include "JoystickInterfaceLinux.h"
#include "log/Log.h"
#include "utils/CommonMacros.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/joystick.h>
#include <sstream>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace JOYSTICK;

#define MAX_AXIS           32767
#define INVALID_FD         -1

CJoystickLinux::CJoystickLinux(int fd, const std::string& strFilename, CJoystickInterfaceLinux* api)
 : CJoystick(api),
   m_fd(fd),
   m_strFilename(strFilename)
{
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

bool CJoystickLinux::Initialize(void)
{
  m_stateBuffer.buttons.assign(ButtonCount(), JOYSTICK_STATE_BUTTON());
  m_stateBuffer.hats.assign(HatCount(), JOYSTICK_STATE_HAT());
  m_stateBuffer.axes.assign(AxisCount(), JOYSTICK_STATE_AXIS());

  return CJoystick::Initialize();
}


void CJoystickLinux::Deinitialize(void)
{
  close(m_fd);
  m_fd = INVALID_FD;

  CJoystick::Deinitialize();
}

bool CJoystickLinux::ScanEvents(std::vector<ADDON::PeripheralEvent>& events)
{
  std::vector<JOYSTICK_STATE_BUTTON>& buttons = m_stateBuffer.buttons;
  std::vector<JOYSTICK_STATE_AXIS>&   axes    = m_stateBuffer.axes;

  ASSERT(buttons.size() == ButtonCount());
  ASSERT(axes.size() == AxisCount());

  ReadEvents(buttons, axes);

  GetButtonEvents(buttons, events);
  GetAxisEvents(axes, events);

  return true;
}

void CJoystickLinux::ReadEvents(std::vector<JOYSTICK_STATE_BUTTON>& buttons,
                                std::vector<JOYSTICK_STATE_AXIS>& axes)
{
  js_event joyEvent;

  while (true)
  {
    // Flush the driver queue
    if (read(m_fd, &joyEvent, sizeof(joyEvent)) != sizeof(joyEvent))
    {
      if (errno == EAGAIN)
      {
        // The circular driver queue holds 64 events. If compiling your own driver,
        // you can increment this size bumping up JS_BUFF_SIZE in joystick.h
        break;
      }
      else
      {
        esyslog("%s: failed to read joystick \"%s\" on %s",
            __FUNCTION__, Name().c_str(), m_strFilename.c_str());
        break;
      }
    }

    // The possible values of joystickEvent.type are:
    // JS_EVENT_BUTTON    0x01    // button pressed/released
    // JS_EVENT_AXIS      0x02    // joystick moved
    // JS_EVENT_INIT      0x80    // (flag) initial state of device

    // Ignore initial events, because they mess up the buttons
    switch (joyEvent.type)
    {
    case JS_EVENT_BUTTON:
      if (joyEvent.number < ButtonCount())
        buttons[joyEvent.number] = joyEvent.value ? JOYSTICK_STATE_BUTTON_PRESSED : JOYSTICK_STATE_BUTTON_UNPRESSED;
      break;
    case JS_EVENT_AXIS:
      if (joyEvent.number < AxisCount())
        axes[joyEvent.number] = NormalizeAxis(joyEvent.value, MAX_AXIS);
      break;
    default:
      break;
    }
  }
}
