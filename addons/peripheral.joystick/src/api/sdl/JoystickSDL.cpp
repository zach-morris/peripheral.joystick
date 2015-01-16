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

#include "JoystickSDL.h"
#include "JoystickInterfaceSDL.h"

#define MAX_AXISAMOUNT    32768

using namespace JOYSTICK;

CJoystickSDL::CJoystickSDL(SDL_Joystick* pJoystick, CJoystickInterfaceSDL* api)
 : CJoystick(api),
   m_pJoystick(pJoystick)
{
}

bool CJoystickSDL::GetEvents(std::vector<ADDON::PeripheralEvent>& events)
{
  std::vector<JOYSTICK_STATE_BUTTON>& buttons = m_stateBuffer.buttons;
  std::vector<JOYSTICK_STATE_HAT>&    hats    = m_stateBuffer.hats;
  std::vector<JOYSTICK_STATE_AXIS>&   axes    = m_stateBuffer.axes;

  // Update the state of all opened joysticks
  SDL_JoystickUpdate();

  // Gamepad buttons
  for (unsigned int b = 0; b < ButtonCount(); b++)
    buttons[b] = SDL_JoystickGetButton(m_pJoystick, b) ? JOYSTICK_STATE_BUTTON_PRESSED : JOYSTICK_STATE_BUTTON_UNPRESSED;

  // Gamepad hats
  for (unsigned int h = 0; h < HatCount(); h++)
  {
    uint8_t hat = SDL_JoystickGetHat(m_pJoystick, h);
    switch (hat)
    {
      case SDL_HAT_UP:
        hats[h] = JOYSTICK_STATE_HAT_UP;
        break;
      case SDL_HAT_RIGHT:
        hats[h] = JOYSTICK_STATE_HAT_RIGHT;
        break;
      case SDL_HAT_DOWN:
        hats[h] = JOYSTICK_STATE_HAT_DOWN;
        break;
      case SDL_HAT_LEFT:
        hats[h] = JOYSTICK_STATE_HAT_LEFT;
        break;
      case SDL_HAT_RIGHTUP:
        hats[h] = JOYSTICK_STATE_HAT_RIGHT_UP;
        break;
      case SDL_HAT_RIGHTDOWN:
        hats[h] = JOYSTICK_STATE_HAT_RIGHT_DOWN;
        break;
      case SDL_HAT_LEFTUP:
        hats[h] = JOYSTICK_STATE_HAT_LEFT_UP;
        break;
      case SDL_HAT_LEFTDOWN:
        hats[h] = JOYSTICK_STATE_HAT_LEFT_DOWN;
        break;
      case SDL_HAT_CENTERED:
      default:
        hats[h] = JOYSTICK_STATE_HAT_UNPRESSED;
        break;
    }
  }

  // Gamepad axes
  for (unsigned int a = 0; a < AxisCount(); a++)
    axes[a] = NormalizeAxis((long)SDL_JoystickGetAxis(m_pJoystick, a), MAX_AXISAMOUNT);

  GetButtonEvents(buttons, events);
  GetHatEvents(hats, events);
  GetAxisEvents(axes, events);

  return true;
}
