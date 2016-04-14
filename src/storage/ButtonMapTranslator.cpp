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
#include "api/JoystickTranslator.h"

#include <cstdlib>
#include <cctype>
#include <sstream>

using namespace JOYSTICK;

#define HAT_CHAR  'h'

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
      strPrimitive << HAT_CHAR;
      strPrimitive << primitive.DriverIndex();
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
    bool bIsButton = std::isdigit(strPrimitive[0]) ? true : false;
    bool bIsHat = (strPrimitive[0] == HAT_CHAR);
    bool bIsAxis = (JoystickTranslator::TranslateSemiAxisDir(strPrimitive[0]) != JOYSTICK_DRIVER_SEMIAXIS_UNKNOWN);

    if (bIsButton)
    {
      primitive = ADDON::DriverPrimitive::CreateButton(std::atoi(strPrimitive.c_str()));
    }
    else if (bIsHat)
    {
      unsigned int hatIndex = std::atoi(strPrimitive.substr(1).c_str());

      size_t dirPos = strPrimitive.find_first_not_of("0123456789", 1);
      if (dirPos != std::string::npos)
      {
        JOYSTICK_DRIVER_HAT_DIRECTION hatDir = JoystickTranslator::TranslateHatDir(strPrimitive.substr(dirPos));

        if (hatDir != JOYSTICK_DRIVER_HAT_UNKNOWN)
          primitive = ADDON::DriverPrimitive(hatIndex, hatDir);
      }
    }
    else if (bIsAxis)
    {
      primitive = ADDON::DriverPrimitive(std::atoi(strPrimitive.substr(1).c_str()),
                                         JoystickTranslator::TranslateSemiAxisDir(strPrimitive[0]));
    }
  }

  return primitive;
}
