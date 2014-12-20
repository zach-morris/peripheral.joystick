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

#include "JoystickInterfaceLinux.h"
#include "JoystickLinux.h"
#include "api/JoystickTypes.h"
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

/*
 * The following values come from include/input.h in the kernel source; the
 * small variant is used up to version 2.6.27, the large one from 2.6.28
 * onwards. We need to handle both values because the kernel doesn't; it only
 * expects one of the values, and we need to determine which one at run-time.
 */
#define KEY_MAX_LARGE 0x2FF
#define KEY_MAX_SMALL 0x1FF

/* Axis map size */
#define AXMAP_SIZE (ABS_MAX + 1)

/* Button map size */
#define BTNMAP_SIZE (KEY_MAX_LARGE - BTN_MISC + 1)

/* The following values come from include/joystick.h in the kernel source */
#define JSIOCSBTNMAP_LARGE _IOW('j', 0x33, __u16[KEY_MAX_LARGE - BTN_MISC + 1])
#define JSIOCSBTNMAP_SMALL _IOW('j', 0x33, __u16[KEY_MAX_SMALL - BTN_MISC + 1])
#define JSIOCGBTNMAP_LARGE _IOR('j', 0x34, __u16[KEY_MAX_LARGE - BTN_MISC + 1])
#define JSIOCGBTNMAP_SMALL _IOR('j', 0x34, __u16[KEY_MAX_SMALL - BTN_MISC + 1])

#define JOYSTICK_UNKNOWN   "Unknown XBMC-Compatible Linux Joystick"
#define MAX_AXIS           32767

using namespace ADDON;
using namespace JOYSTICK;

static const char *axis_names[ABS_MAX + 1] =
{
  "X",     "Y",     "Z",     "Rx",    "Ry",    "Rz",    "Throttle", "Rudder",
  "Wheel", "Gas",   "Brake", "?",     "?",     "?",     "?",        "?",
  "Hat0X", "Hat0Y", "Hat1X", "Hat1Y", "Hat2X", "Hat2Y", "Hat3X",    "Hat3Y",
  "?",     "?",     "?",      "?",    "?",     "?",     "?",
};

static const char *button_names[KEY_MAX - BTN_MISC + 1] =
{
  "Btn0",       "Btn1",     "Btn2",      "Btn3",      "Btn4",      "Btn5",     "Btn6",
  "Btn7",       "Btn8",     "Btn9",      "?",         "?",         "?",        "?",
  "?",          "?",        "LeftBtn",   "RightBtn",  "MiddleBtn", "SideBtn",  "ExtraBtn",
  "ForwardBtn", "BackBtn",  "TaskBtn",   "?",         "?",         "?",        "?",
  "?",          "?",        "?",         "?",         "Trigger",   "ThumbBtn", "ThumbBtn2",
  "TopBtn",     "TopBtn2",  "PinkieBtn", "BaseBtn",   "BaseBtn2",  "BaseBtn3", "BaseBtn4",
  "BaseBtn5",   "BaseBtn6", "BtnDead",   "BtnA",      "BtnB",      "BtnC",     "BtnX",
  "BtnY",       "BtnZ",     "BtnTL",     "BtnTR",     "BtnTL2",    "BtnTR2",   "BtnSelect",
  "BtnStart",   "BtnMode",  "BtnThumbL", "BtnThumbR", "?",         "?",        "?",
  "?",          "?",        "?",         "?",         "?",         "?",        "?",
  "?",          "?",        "?",         "?",         "?",         "?",        "?",
  "WheelBtn",   "Gear up",
};

CJoystickInterfaceLinux::CJoystickInterfaceLinux(void)
 : CJoystickInterface(INTERFACE_LINUX)
{
}

bool CJoystickInterfaceLinux::PerformJoystickScan(std::vector<CJoystick*>& joysticks)
{
  Deinitialize();

  // TODO: Use udev to grab device names instead of reading /dev/input/js*
  std::string inputDir("/dev/input");
  DIR *pd = opendir(inputDir.c_str());
  if (pd == NULL)
  {
    esyslog("%s: can't open %s (errno=%d)", __FUNCTION__, inputDir.c_str(), errno);
    return PERIPHERAL_ERROR_FAILED;
  }

  dirent *pDirent;
  while ((pDirent = readdir(pd)) != NULL)
  {
    if (std::string(pDirent->d_name).substr(0, 2) == "js")
    {
      // Found a joystick device
      std::string filename(inputDir + "/" + pDirent->d_name);
      isyslog("CJoystickInterfaceLinux::Initialize: opening joystick %s", filename.c_str());

      int fd = open(filename.c_str(), O_RDONLY);
      if (fd < 0)
      {
        esyslog("%s: can't open %s (errno=%d)", __FUNCTION__, filename.c_str(), errno);
        continue;
      }

      unsigned char axes = 0;
      unsigned char buttons = 0;
      int version = 0x000000;
      char name[128] = JOYSTICK_UNKNOWN;

      if (ioctl(fd, JSIOCGVERSION, &version) < 0 ||
          ioctl(fd, JSIOCGAXES, &axes)       < 0 ||
          ioctl(fd, JSIOCGBUTTONS, &buttons) < 0 ||
          ioctl(fd, JSIOCGNAME(128), name)   < 0)
      {
        esyslog("%s: failed ioctl() (errno=%d)", __FUNCTION__, errno);
        close(fd);
        continue;
      }

      if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
      {
        esyslog("%s: failed fcntl() (errno=%d)", __FUNCTION__, errno);
        close(fd);
        continue;
      }

      // We don't support the old (0.x) interface
      if (version < 0x010000)
      {
        esyslog("%s: old (0.x) interface is not supported (version=%08x)", __FUNCTION__, version);
        close(fd);
        continue;
      }

      isyslog("%s: Enabled Joystick: \"%s\" (Linux Joystick API)", __FUNCTION__, name);
      isyslog("%s: driver version is %d.%d.%d", __FUNCTION__,
          version >> 16, (version >> 8) & 0xff, version & 0xff);

      uint16_t buttonMap[BTNMAP_SIZE];
      uint8_t axisMap[AXMAP_SIZE];

      if (!GetButtonMap(fd, buttonMap) || !GetAxisMap(fd, axisMap))
      {
        esyslog("%s: can't get button or axis map", __FUNCTION__);
        // I assume this isn't a fatal error...
      }

      // Determine whether the button map is usable
      bool buttonMapOK = true;
      for (int i = 0; buttonMapOK && i < buttons; i++)
      {
        if (buttonMap[i] < BTN_MISC || buttonMap[i] > KEY_MAX)
        {
          buttonMapOK = false;
          break;
        }
      }

      if (!buttonMapOK)
      {
        // buttonMap out of range for names. Don't print any.
        esyslog("%s: XBMC is not fully compatible with your kernel. Unable to retrieve button map!",
            __FUNCTION__);
        isyslog("%s: Joystick \"%s\" has %d buttons and %d axes", __FUNCTION__,
            name, buttons, axes);
      }
      else
      {
        std::ostringstream strButtons;
        for (int i = 0; i < buttons; i++)
        {
          strButtons << button_names[buttonMap[i] - BTN_MISC];
          if (i < buttons - 1)
            strButtons << ", ";
        }
        isyslog("Buttons: %s", strButtons.str().c_str());

        std::ostringstream strAxes;
        for (int i = 0; i < axes; i++)
        {
          strAxes << axis_names[axisMap[i]];
          if (i < axes - 1)
            strAxes << ", ";
        }
        isyslog("Axes: %s", strAxes.str().c_str());
      }

      CJoystick* joystick = new CJoystickLinux(fd, filename, this);
      joystick->SetName(name);
      joystick->SetButtonCount(buttons);
      joystick->SetAxisCount(axes);
      joysticks.push_back(joystick);
    }
  }

  closedir(pd);

  return PERIPHERAL_NO_ERROR;
}

bool CJoystickInterfaceLinux::GetButtonMap(int fd, uint16_t *buttonMap)
{
  static unsigned long joyGetButtonMapIoctl = 0;
  static const unsigned long ioctls[] = { JSIOCGBTNMAP, JSIOCGBTNMAP_LARGE, JSIOCGBTNMAP_SMALL, 0 };

  if (joyGetButtonMapIoctl == 0)
  {
    return DetermineIoctl(fd, ioctls, buttonMap, joyGetButtonMapIoctl);
  }
  else
  {
    // We already know which ioctl to use
    return ioctl(fd, joyGetButtonMapIoctl, buttonMap) >= 0;
  }
}

bool CJoystickInterfaceLinux::GetAxisMap(int fd, uint8_t *axisMap)
{
  return ioctl(fd, JSIOCGAXMAP, axisMap);
}

bool CJoystickInterfaceLinux::DetermineIoctl(int fd, const unsigned long *ioctls, uint16_t *buttonMap, unsigned long &ioctl_used)
{
  int retval = 0;

  // Try each ioctl in turn
  for (int i = 0; ioctls[i] != 0; i++)
  {
    retval = ioctl(fd, ioctls[i], (void*)buttonMap);
    if (retval >= 0)
    {
      // The ioctl did something
      ioctl_used = ioctls[i];
      return true;
    }
    else if (errno != -EINVAL)
    {
      // Some other error occurred
      return false;
    }
  }
  return false;
}
