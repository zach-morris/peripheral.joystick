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

#include "JoystickInterfaceSDL.h"
#include "JoystickSDL.h"
#include "api/JoystickTypes.h"
#include "log/Log.h"

#include <SDL/SDL.h>
#include <SDL/SDL_joystick.h>

using namespace JOYSTICK;

CJoystickInterfaceSDL::CJoystickInterfaceSDL(void)
 : CJoystickInterface(INTERFACE_SDL)
{
}

void CJoystickInterfaceSDL::Deinitialize(void)
{
  // Restart SDL joystick subsystem
  SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
  if (SDL_WasInit(SDL_INIT_JOYSTICK) !=  0)
    esyslog("Stopping joystick SDL subsystem failed");
}

bool CJoystickInterfaceSDL::PerformJoystickScan(std::vector<CJoystick*>& joysticks)
{
  Deinitialize();

  if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) != 0)
  {
    esyslog("(Re)start joystick subsystem failed : %s", SDL_GetError());
    return false;
  }

  // Any joysticks connected?
  if (SDL_NumJoysticks() > 0)
  {
    // Load joystick names and open all connected joysticks
    for (int i = 0 ; i < SDL_NumJoysticks(); i++)
    {
      SDL_Joystick *joy = SDL_JoystickOpen(i);
#if defined(TARGET_DARWIN)
      // On OS X, the 360 controllers are handled externally, since the SDL code is
      // really buggy and doesn't handle disconnects.
      if (std::string(SDL_JoystickName(i)).find("360") != std::string::npos)
      {
        isyslog("Ignoring joystick: %s", SDL_JoystickName(i));
        continue;
      }
#endif
      if (joy)
      {
	    // Some (Microsoft) Keyboards are recognized as Joysticks by modern kernels
		// Don't enumerate them
        // https://bugs.launchpad.net/ubuntu/+source/linux/+bug/390959
        // NOTICE: Enabled Joystick: Microsoft Wired Keyboard 600
        // Details: Total Axis: 37 Total Hats: 0 Total Buttons: 57
        // NOTICE: Enabled Joystick: Microsoft Microsoft® 2.4GHz Transceiver v6.0
        // Details: Total Axis: 37 Total Hats: 0 Total Buttons: 57
        int num_axis = SDL_JoystickNumAxes(joy);
        int num_buttons = SDL_JoystickNumButtons(joy);
        if (num_axis > 20 && num_buttons > 50)
        {
          isyslog("Your Joystick seems to be a Keyboard, ignoring it: %s Axis: %d Buttons: %d",
            SDL_JoystickName(i), num_axis, num_buttons);
        }
        else
        {
          CJoystick* joystick = new CJoystickSDL(joy, this);
          joystick->SetName(SDL_JoystickName(i));
          joystick->SetButtonCount(SDL_JoystickNumButtons(joy));
          joystick->SetHatCount(SDL_JoystickNumHats(joy));
          joystick->SetAxisCount(SDL_JoystickNumAxes(joy));
          joysticks.push_back(joystick);

          /* TODO
          isyslog("Enabled Joystick: \"%s\" (SDL)", joystick.m_configuration.Name().c_str());
          isyslog("Details: Total Axes: %d Total Hats: %d Total Buttons: %d",
                  joystick.m_configuration.AxisIndexes().size(),
                  joystick.m_configuration.HatIndexes().size(),
                  joystick.m_configuration.ButtonIndexes().size());;
          */
        }
      }
    }
  }

  // Disable joystick events, since we'll be polling them
  SDL_JoystickEventState(SDL_DISABLE);
  
  return true;
}
