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

#include "ButtonMapper.h"

using namespace JOYSTICK;

CButtonMapper& CButtonMapper::Get(void)
{
  static CButtonMapper _instance;
  return _instance;
}

bool CButtonMapper::GetFeatures(const ADDON::Peripheral& peripheral, const ADDON::Joystick& joystick,
                                const std::string& strDevice, std::vector<ADDON::JoystickFeature*>& features) const
{
  const std::string& strName     = peripheral.Name();
  uint16_t           vid         = peripheral.VendorID();
  uint16_t           pid         = peripheral.ProductID();
  const std::string& strProvider = joystick.Provider();
  unsigned int       buttonCount = joystick.ButtonCount();
  unsigned int       hatCount    = joystick.HatCount();
  unsigned int       axisCount   = joystick.AxisCount();

  std::map<std::string, DeviceButtonMaps>::const_iterator itJoystick = m_joysticks.find(strName);
  if (itJoystick != m_joysticks.end())
  {
    const DeviceButtonMaps& deviceButtonMaps = itJoystick->second;
    DeviceButtonMaps::const_iterator itButtonMap = deviceButtonMaps.find(strDevice);
    if (itButtonMap != deviceButtonMaps.end())
    {
      const ButtonMap& buttonMap = itButtonMap->second;
      for (ButtonMap::const_iterator itFeature = buttonMap.begin(); itFeature != buttonMap.end(); ++itFeature)
        features.push_back(itFeature->second);
    }
  }

  return true;
}

bool CButtonMapper::MapFeature(const ADDON::Peripheral& peripheral, const ADDON::Joystick& joystick,
                               const std::string& strDevice, const ADDON::JoystickFeature* feature)
{
  const std::string& strName     = peripheral.Name();
  uint16_t           vid         = peripheral.VendorID();
  uint16_t           pid         = peripheral.ProductID();
  const std::string& strProvider = joystick.Provider();
  unsigned int       buttonCount = joystick.ButtonCount();
  unsigned int       hatCount    = joystick.HatCount();
  unsigned int       axisCount   = joystick.AxisCount();

  ButtonMap& buttonMap = m_joysticks[strName][strDevice];

  UnMap(buttonMap, feature);

  ButtonMap::iterator itFeature = buttonMap.find(feature->ID());
  if (itFeature != buttonMap.end())
    delete itFeature->second;

  buttonMap[feature->ID()] = feature->Clone();

  return true;
}

void CButtonMapper::UnMap(ButtonMap& buttonMap, const ADDON::JoystickFeature* feature)
{
  switch (feature->Type())
  {
    case JOYSTICK_DRIVER_TYPE_BUTTON:
      UnMapButton(buttonMap, static_cast<const ADDON::DriverButton*>(feature));
      break;
    case JOYSTICK_DRIVER_TYPE_HAT_DIRECTION:
      UnMapHat(buttonMap, static_cast<const ADDON::DriverHat*>(feature));
      break;
    case JOYSTICK_DRIVER_TYPE_SEMIAXIS:
      UnMapSemiAxis(buttonMap, static_cast<const ADDON::DriverSemiAxis*>(feature));
      break;
    case JOYSTICK_DRIVER_TYPE_ANALOG_STICK:
      UnMapAnalogStick(buttonMap, static_cast<const ADDON::DriverAnalogStick*>(feature));
      break;
    case JOYSTICK_DRIVER_TYPE_ACCELEROMETER:
      UnMapAccelerometer(buttonMap, static_cast<const ADDON::DriverAccelerometer*>(feature));
      break;
    default:
      break;
  }
}

void CButtonMapper::UnMapButton(ButtonMap& buttonMap, const ADDON::DriverButton* button)
{
  bool bConflicted = false;

  for (ButtonMap::iterator it = buttonMap.begin(); !bConflicted && it != buttonMap.end(); ++it)
  {
    if (ButtonConflicts(button, it->second))
    {
      bConflicted = true;
      buttonMap.erase(it);
    }
  }
}

void CButtonMapper::UnMapHat(ButtonMap& buttonMap, const ADDON::DriverHat* hat)
{
  bool bConflicted = false;

  for (ButtonMap::iterator it = buttonMap.begin(); !bConflicted && it != buttonMap.end(); ++it)
  {
    if (HatConflicts(hat, it->second))
    {
      bConflicted = true;
      buttonMap.erase(it);
    }
  }
}

void CButtonMapper::UnMapSemiAxis(ButtonMap& buttonMap, const ADDON::DriverSemiAxis* semiAxis)
{
  bool bConflicted = false;

  for (ButtonMap::iterator it = buttonMap.begin(); !bConflicted && it != buttonMap.end(); ++it)
  {
    if (SemiAxisConflicts(semiAxis, it->second))
    {
      bConflicted = true;

      switch (it->second->Type())
      {
        case JOYSTICK_DRIVER_TYPE_SEMIAXIS:
        {
          buttonMap.erase(it);
          break;
        }
        case JOYSTICK_DRIVER_TYPE_ANALOG_STICK:
        {
          ADDON::DriverAnalogStick* analogStick = static_cast<ADDON::DriverAnalogStick*>(it->second);

          if (semiAxis->Index() == analogStick->XIndex())
          {
            analogStick->SetXIndex(-1);
            if (analogStick->YIndex() < 0)
              buttonMap.erase(it);
          }
          else if (semiAxis->Index() == analogStick->YIndex())
          {
            analogStick->SetYIndex(-1);
            if (analogStick->XIndex() < 0)
              buttonMap.erase(it);
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
              buttonMap.erase(it);
          }
          else if (semiAxis->Index() == accelerometer->YIndex())
          {
            accelerometer->SetYIndex(-1);
            if (accelerometer->XIndex() < 0 && accelerometer->ZIndex() < 0)
              buttonMap.erase(it);
          }
          else if (semiAxis->Index() == accelerometer->ZIndex())
          {
            accelerometer->SetZIndex(-1);
            if (accelerometer->XIndex() < 0 && accelerometer->YIndex() < 0)
              buttonMap.erase(it);
          }
          break;
        }
        default:
          break;
      }
    }
  }
}

void CButtonMapper::UnMapAnalogStick(ButtonMap& buttonMap, const ADDON::DriverAnalogStick* analogStick)
{
  ADDON::DriverSemiAxis semiAxis;

  if (analogStick->XIndex() >= 0)
  {
    semiAxis.SetIndex(analogStick->XIndex());
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE);
    UnMapSemiAxis(buttonMap, &semiAxis);
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE);
    UnMapSemiAxis(buttonMap, &semiAxis);
  }

  if (analogStick->YIndex() >= 0)
  {
    semiAxis.SetIndex(analogStick->YIndex());
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE);
    UnMapSemiAxis(buttonMap, &semiAxis);
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE);
    UnMapSemiAxis(buttonMap, &semiAxis);
  }
}

void CButtonMapper::UnMapAccelerometer(ButtonMap& buttonMap, const ADDON::DriverAccelerometer* accelerometer)
{
  ADDON::DriverSemiAxis semiAxis;

  if (accelerometer->XIndex() >= 0)
  {
    semiAxis.SetIndex(accelerometer->XIndex());
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE);
    UnMapSemiAxis(buttonMap, &semiAxis);
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE);
    UnMapSemiAxis(buttonMap, &semiAxis);
  }

  if (accelerometer->YIndex() >= 0)
  {
    semiAxis.SetIndex(accelerometer->YIndex());
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE);
    UnMapSemiAxis(buttonMap, &semiAxis);
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE);
    UnMapSemiAxis(buttonMap, &semiAxis);
  }

  if (accelerometer->ZIndex() >= 0)
  {
    semiAxis.SetIndex(accelerometer->ZIndex());
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE);
    UnMapSemiAxis(buttonMap, &semiAxis);
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE);
    UnMapSemiAxis(buttonMap, &semiAxis);
  }
}

bool CButtonMapper::ButtonConflicts(const ADDON::DriverButton* button, const ADDON::JoystickFeature* feature)
{
  if (feature->Type() == JOYSTICK_DRIVER_TYPE_BUTTON)
  {
    const ADDON::DriverButton* button2 = static_cast<const ADDON::DriverButton*>(feature);
    return button->Index() == button2->Index();
  }
  return false;
}

bool CButtonMapper::HatConflicts(const ADDON::DriverHat* hat, const ADDON::JoystickFeature* feature)
{
  if (feature->Type() == JOYSTICK_DRIVER_TYPE_HAT_DIRECTION)
  {
    const ADDON::DriverHat* hat2 = static_cast<const ADDON::DriverHat*>(feature);
    return hat->Index()     == hat2->Index()  &&
           hat->Direction() == hat2->Direction();
  }
  return false;
}

bool CButtonMapper::SemiAxisConflicts(const ADDON::DriverSemiAxis* semiAxis, const ADDON::JoystickFeature* feature)
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
