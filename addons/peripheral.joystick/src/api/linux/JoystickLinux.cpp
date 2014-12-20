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
}

void CJoystickLinux::Deinitialize(void)
{
  close(m_fd);
  m_fd = INVALID_FD;
}

bool CJoystickLinux::GetEvents(std::vector<ADDON::PeripheralEvent>& events)
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

    /* TODO

    // The possible values of joystickEvent.type are:
    // JS_EVENT_BUTTON    0x01    // button pressed/released
    // JS_EVENT_AXIS      0x02    // joystick moved
    // JS_EVENT_INIT      0x80    // (flag) initial state of device

    // Ignore initial events, because they mess up the buttons
    switch (joyEvent.type)
    {
    case JS_EVENT_BUTTON:
      if (joyEvent.number < m_state.buttons.size())
        m_state.buttons[joyEvent.number] = joyEvent.value;
      break;
    case JS_EVENT_AXIS:
      if (joyEvent.number < m_state.axes.size())
        m_state.SetAxis(joyEvent.number, joyEvent.value, MAX_AXIS);
      break;
    default:
      break;
    }
    */
  }

  return false;
}
