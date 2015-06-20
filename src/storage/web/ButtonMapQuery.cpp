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

#include "ButtonMapQuery.h"
#include "JoystickDefinitions.h"
#include "JoystickTranslator.h"

#include <sstream>

using namespace JOYSTICK;

void CButtonMapQuery::GetQueryString(std::stringstream& ss) const
{
  for (Buttons::const_iterator it = m_buttons.begin(); it != m_buttons.end(); ++it)
  {
    const std::string& strFeatureName = it->first;
    const ADDON::JoystickFeature* feature = it->second;

    ss << "&" << strFeatureName << "=";

    switch (feature->Type())
    {
      case JOYSTICK_DRIVER_TYPE_BUTTON:
      {
        const ADDON::DriverButton* button = static_cast<const ADDON::DriverButton*>(feature);
        ss << button->Index();
        break;
      }
      case JOYSTICK_DRIVER_TYPE_HAT_DIRECTION:
      {
        const ADDON::DriverHat* hat = static_cast<const ADDON::DriverHat*>(feature);
        ss << JoystickTranslator::TranslateHatDir(hat->Direction());
        break;
      }
      case JOYSTICK_DRIVER_TYPE_SEMIAXIS:
      {
        const ADDON::DriverSemiAxis* semiAxis = static_cast<const ADDON::DriverSemiAxis*>(feature);
        if (semiAxis->Direction() == JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE)
          ss << "-";
        else
          ss << "+";
        ss << semiAxis->Index();
        break;
      }
      case JOYSTICK_DRIVER_TYPE_ANALOG_STICK:
      {
        const ADDON::DriverAnalogStick* analogStick = static_cast<const ADDON::DriverAnalogStick*>(feature);
        if (analogStick->XInverted())
          ss << "-";
        else
          ss << "+";
        ss << analogStick->XIndex() << ",";
        if (analogStick->YInverted())
          ss << "-";
        else
          ss << "+";
        ss << analogStick->YIndex();
        break;
      }
      case JOYSTICK_DRIVER_TYPE_ACCELEROMETER:
      {
        const ADDON::DriverAccelerometer* accelerometer = static_cast<const ADDON::DriverAccelerometer*>(feature);
        if (accelerometer->XInverted())
          ss << "-";
        else
          ss << "+";
        ss << accelerometer->XIndex() << ",";
        if (accelerometer->YInverted())
          ss << "-";
        else
          ss << "+";
        ss << accelerometer->YIndex() << ",";
        if (accelerometer->ZInverted())
          ss << "-";
        else
          ss << "+";
        ss << accelerometer->ZIndex();
        break;
      }
      default:
        break;
    }
  }
}
