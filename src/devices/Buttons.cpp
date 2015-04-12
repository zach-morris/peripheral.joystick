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

#include "Buttons.h"

using namespace JOYSTICK;

bool CButtons::GetFeatures(std::vector<ADDON::JoystickFeature*>& features) const
{
  for (Buttons::const_iterator itButton = m_buttons.begin(); itButton != m_buttons.end(); ++itButton)
    features.push_back(itButton->second);

  return true;
}

bool CButtons::MapFeature(const ADDON::JoystickFeature* feature)
{
  if (feature && !feature->Name().empty())
  {
    UnMap(feature);

    const std::string& strFeatureName = feature->Name();

    Buttons::iterator itFeature = m_buttons.find(strFeatureName);
    if (itFeature != m_buttons.end())
      delete itFeature->second;

    m_buttons[strFeatureName] = feature->Clone();

    return true;
  }

  return false;
}

void CButtons::UnMap(const ADDON::JoystickFeature* feature)
{
  switch (feature->Type())
  {
    case JOYSTICK_DRIVER_TYPE_BUTTON:
      UnMapButton(static_cast<const ADDON::DriverButton*>(feature));
      break;
    case JOYSTICK_DRIVER_TYPE_HAT_DIRECTION:
      UnMapHat(static_cast<const ADDON::DriverHat*>(feature));
      break;
    case JOYSTICK_DRIVER_TYPE_SEMIAXIS:
      UnMapSemiAxis(static_cast<const ADDON::DriverSemiAxis*>(feature));
      break;
    case JOYSTICK_DRIVER_TYPE_ANALOG_STICK:
      UnMapAnalogStick(static_cast<const ADDON::DriverAnalogStick*>(feature));
      break;
    case JOYSTICK_DRIVER_TYPE_ACCELEROMETER:
      UnMapAccelerometer(static_cast<const ADDON::DriverAccelerometer*>(feature));
      break;
    default:
      break;
  }
}

void CButtons::UnMapButton(const ADDON::DriverButton* button)
{
  for (Buttons::iterator it = m_buttons.begin(); it != m_buttons.end(); ++it)
  {
    if (ButtonConflicts(button, it->second))
    {
      m_buttons.erase(it);
      break;
    }
  }
}

void CButtons::UnMapHat(const ADDON::DriverHat* hat)
{
  for (Buttons::iterator it = m_buttons.begin(); it != m_buttons.end(); ++it)
  {
    if (HatConflicts(hat, it->second))
    {
      m_buttons.erase(it);
      break;
    }
  }
}

void CButtons::UnMapSemiAxis(const ADDON::DriverSemiAxis* semiAxis)
{
  for (Buttons::iterator it = m_buttons.begin(); it != m_buttons.end(); ++it)
  {
    if (SemiAxisConflicts(semiAxis, it->second))
    {
      switch (it->second->Type())
      {
        case JOYSTICK_DRIVER_TYPE_SEMIAXIS:
        {
          m_buttons.erase(it);
          break;
        }
        case JOYSTICK_DRIVER_TYPE_ANALOG_STICK:
        {
          ADDON::DriverAnalogStick* analogStick = static_cast<ADDON::DriverAnalogStick*>(it->second);

          if (semiAxis->Index() == analogStick->XIndex())
          {
            analogStick->SetXIndex(-1);
            if (analogStick->YIndex() < 0)
              m_buttons.erase(it);
          }
          else if (semiAxis->Index() == analogStick->YIndex())
          {
            analogStick->SetYIndex(-1);
            if (analogStick->XIndex() < 0)
              m_buttons.erase(it);
          }
          break;
        }
        case JOYSTICK_DRIVER_TYPE_ACCELEROMETER:
        {
          ADDON::DriverAccelerometer* accelerometer = static_cast<ADDON::DriverAccelerometer*>(it->second);

          if (semiAxis->Index() == accelerometer->XIndex())
          {
            accelerometer->SetXIndex(-1);
            if (accelerometer->YIndex() < 0 && accelerometer->ZIndex() < 0)
              m_buttons.erase(it);
          }
          else if (semiAxis->Index() == accelerometer->YIndex())
          {
            accelerometer->SetYIndex(-1);
            if (accelerometer->XIndex() < 0 && accelerometer->ZIndex() < 0)
              m_buttons.erase(it);
          }
          else if (semiAxis->Index() == accelerometer->ZIndex())
          {
            accelerometer->SetZIndex(-1);
            if (accelerometer->XIndex() < 0 && accelerometer->YIndex() < 0)
              m_buttons.erase(it);
          }
          break;
        }
        default:
          break;
      }
    }

    break;
  }
}

void CButtons::UnMapAnalogStick(const ADDON::DriverAnalogStick* analogStick)
{
  ADDON::DriverSemiAxis semiAxis;

  if (analogStick->XIndex() >= 0)
  {
    semiAxis.SetIndex(analogStick->XIndex());
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE);
    UnMapSemiAxis(&semiAxis);
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE);
    UnMapSemiAxis(&semiAxis);
  }

  if (analogStick->YIndex() >= 0)
  {
    semiAxis.SetIndex(analogStick->YIndex());
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE);
    UnMapSemiAxis(&semiAxis);
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE);
    UnMapSemiAxis(&semiAxis);
  }
}

void CButtons::UnMapAccelerometer(const ADDON::DriverAccelerometer* accelerometer)
{
  ADDON::DriverSemiAxis semiAxis;

  if (accelerometer->XIndex() >= 0)
  {
    semiAxis.SetIndex(accelerometer->XIndex());
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE);
    UnMapSemiAxis(&semiAxis);
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE);
    UnMapSemiAxis(&semiAxis);
  }

  if (accelerometer->YIndex() >= 0)
  {
    semiAxis.SetIndex(accelerometer->YIndex());
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE);
    UnMapSemiAxis(&semiAxis);
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE);
    UnMapSemiAxis(&semiAxis);
  }

  if (accelerometer->ZIndex() >= 0)
  {
    semiAxis.SetIndex(accelerometer->ZIndex());
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE);
    UnMapSemiAxis(&semiAxis);
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE);
    UnMapSemiAxis(&semiAxis);
  }
}

bool CButtons::ButtonConflicts(const ADDON::DriverButton* button, const ADDON::JoystickFeature* feature)
{
  if (feature->Type() == JOYSTICK_DRIVER_TYPE_BUTTON)
  {
    const ADDON::DriverButton* button2 = static_cast<const ADDON::DriverButton*>(feature);
    return button->Index() == button2->Index();
  }
  return false;
}

bool CButtons::HatConflicts(const ADDON::DriverHat* hat, const ADDON::JoystickFeature* feature)
{
  if (feature->Type() == JOYSTICK_DRIVER_TYPE_HAT_DIRECTION)
  {
    const ADDON::DriverHat* hat2 = static_cast<const ADDON::DriverHat*>(feature);
    return hat->Index()     == hat2->Index()  &&
           hat->Direction() == hat2->Direction();
  }
  return false;
}

bool CButtons::SemiAxisConflicts(const ADDON::DriverSemiAxis* semiAxis, const ADDON::JoystickFeature* feature)
{
  if (feature->Type() == JOYSTICK_DRIVER_TYPE_SEMIAXIS)
  {
    const ADDON::DriverSemiAxis* semiAxis2 = static_cast<const ADDON::DriverSemiAxis*>(feature);
    return semiAxis->Index()     == semiAxis2->Index()  &&
           semiAxis->Direction() == semiAxis2->Direction();
  }
  else if (feature->Type() == JOYSTICK_DRIVER_TYPE_ANALOG_STICK)
  {
    const ADDON::DriverAnalogStick* analogStick = static_cast<const ADDON::DriverAnalogStick*>(feature);
    return semiAxis->Index() == analogStick->XIndex() ||
           semiAxis->Index() == analogStick->YIndex();
  }
  else if (feature->Type() == JOYSTICK_DRIVER_TYPE_ACCELEROMETER)
  {
    const ADDON::DriverAccelerometer* accelerometer = static_cast<const ADDON::DriverAccelerometer*>(feature);
    return semiAxis->Index() == accelerometer->XIndex() ||
           semiAxis->Index() == accelerometer->YIndex() ||
           semiAxis->Index() == accelerometer->ZIndex();
  }
  return false;
}
