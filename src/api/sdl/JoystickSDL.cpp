/*
 *      Copyright (C) 2014-2015 Garrett Brown
 *      Copyright (C) 2014-2015 Team XBMC
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

#include "JoystickSDL.h"
#include "JoystickInterfaceSDL.h"
#include "api/JoystickTypes.h"
#include "utils/CommonMacros.h"

#define MAX_AXISAMOUNT    32768

using namespace JOYSTICK;

CJoystickSDL::CJoystickSDL(const std::string& strName, SDL_Joystick* pJoystick)
 : CJoystick(INTERFACE_SDL),
   m_pJoystick(pJoystick)
{
  SetName(strName);
  SetButtonCount(SDL_JoystickNumButtons(m_pJoystick));
  SetHatCount(SDL_JoystickNumHats(m_pJoystick));
  SetAxisCount(SDL_JoystickNumAxes(m_pJoystick));
}

CJoystickSDL::~CJoystickSDL(void)
{
  SDL_JoystickClose(m_pJoystick); // joystick pointer is refcounted
}

bool CJoystickSDL::ScanEvents(void)
{
  // Update the state of all opened joysticks
  SDL_JoystickUpdate();

  // Gamepad buttons
  for (unsigned int b = 0; b < ButtonCount(); b++)
    SetButtonValue(b, SDL_JoystickGetButton(m_pJoystick, b) ? JOYSTICK_STATE_BUTTON_PRESSED :
                                                              JOYSTICK_STATE_BUTTON_UNPRESSED);

  // Gamepad hats
  for (unsigned int h = 0; h < HatCount(); h++)
  {
    switch (SDL_JoystickGetHat(m_pJoystick, h))
    {
      case SDL_HAT_UP:
        SetHatValue(h, JOYSTICK_STATE_HAT_UP);
        break;
      case SDL_HAT_RIGHT:
        SetHatValue(h, JOYSTICK_STATE_HAT_RIGHT);
        break;
      case SDL_HAT_DOWN:
        SetHatValue(h, JOYSTICK_STATE_HAT_DOWN);
        break;
      case SDL_HAT_LEFT:
        SetHatValue(h, JOYSTICK_STATE_HAT_LEFT);
        break;
      case SDL_HAT_RIGHTUP:
        SetHatValue(h, JOYSTICK_STATE_HAT_RIGHT_UP);
        break;
      case SDL_HAT_RIGHTDOWN:
        SetHatValue(h, JOYSTICK_STATE_HAT_RIGHT_DOWN);
        break;
      case SDL_HAT_LEFTUP:
        SetHatValue(h, JOYSTICK_STATE_HAT_LEFT_UP);
        break;
      case SDL_HAT_LEFTDOWN:
        SetHatValue(h, JOYSTICK_STATE_HAT_LEFT_DOWN);
        break;
      case SDL_HAT_CENTERED:
      default:
        SetHatValue(h, JOYSTICK_STATE_HAT_UNPRESSED);
        break;
    }
  }

  // Gamepad axes
  for (unsigned int a = 0; a < AxisCount(); a++)
    SetAxisValue(a, (long)SDL_JoystickGetAxis(m_pJoystick, a), MAX_AXISAMOUNT);

  return true;
}
