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
  // Update the state of all opened joysticks
  SDL_JoystickUpdate();

  /* TODO
  // Gamepad buttons
  for (unsigned int b = 0; b < state.buttons.size(); b++)
    state.buttons[b] = (SDL_JoystickGetButton(it->m_pJoystick, b) ? true : false);

  // Gamepad hats
  for (unsigned int h = 0; h < state.hats.size(); h++)
  {
    state.hats[h].Center();
    uint8_t hat = SDL_JoystickGetHat(it->m_pJoystick, h);
    if      (hat & SDL_HAT_UP)    state.hats[h][CJoystickHat::UP] = true;
    else if (hat & SDL_HAT_DOWN)  state.hats[h][CJoystickHat::DOWN] = true;
    if      (hat & SDL_HAT_RIGHT) state.hats[h][CJoystickHat::RIGHT] = true;
    else if (hat & SDL_HAT_LEFT)  state.hats[h][CJoystickHat::LEFT] = true;
  }

  // Gamepad axes
  for (unsigned int a = 0; a < state.axes.size(); a++)
    state.SetAxis(a, (long)SDL_JoystickGetAxis(it->m_pJoystick, a), MAX_AXISAMOUNT);
  */

  return false;
}
