/*
 *      Copyright (C) 2015 Garrett Brown
 *      Copyright (C) 2015 Team XBMC
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

#include "ButtonMapTranslator.h"
#include "JoystickTranslator.h"

#include <cstdlib>
#include <sstream>

using namespace JOYSTICK;

std::string ButtonMapTranslator::ToString(const ADDON::DriverPrimitive& primitive)
{
  std::stringstream strPrimitive;
  switch (primitive.Type())
  {
    case JOYSTICK_DRIVER_PRIMITIVE_TYPE_BUTTON:
    {
      strPrimitive << primitive.DriverIndex();
      break;
    }
    case JOYSTICK_DRIVER_PRIMITIVE_TYPE_HAT_DIRECTION:
    {
      strPrimitive << JoystickTranslator::TranslateHatDir(primitive.HatDirection());
      break;
    }
    case JOYSTICK_DRIVER_PRIMITIVE_TYPE_SEMIAXIS:
    {
      const char* dir = JoystickTranslator::TranslateSemiAxisDir(primitive.SemiAxisDirection());
      if (*dir != '\0')
      {
        strPrimitive << dir;
        strPrimitive << primitive.DriverIndex();
      }
      break;
    }
    default:
      break;
  }
  return strPrimitive.str();
}

ADDON::DriverPrimitive ButtonMapTranslator::ToDriverPrimitive(const std::string& strPrimitive)
{
  ADDON::DriverPrimitive primitive;

  if (!strPrimitive.empty())
  {
    JOYSTICK_DRIVER_SEMIAXIS_DIRECTION dir = JoystickTranslator::TranslateSemiAxisDir(strPrimitive[0]);
    if (dir != JOYSTICK_DRIVER_SEMIAXIS_UNKNOWN)
    {
      primitive = ADDON::DriverPrimitive(std::atoi(strPrimitive.substr(1).c_str()), dir);
    }
    else
    {
      JOYSTICK_DRIVER_HAT_DIRECTION hatDir = JoystickTranslator::TranslateHatDir(strPrimitive);
      if (hatDir != JOYSTICK_DRIVER_HAT_UNKNOWN)
      {
        primitive = ADDON::DriverPrimitive(0, hatDir);
      }
      else
      {
        primitive = ADDON::DriverPrimitive(std::atoi(strPrimitive.c_str()));
      }
    }
  }
  return primitive;
}
