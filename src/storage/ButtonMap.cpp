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

#include "ButtonMap.h"
#include "log/Log.h"

using namespace JOYSTICK;

CButtonMap& CButtonMap::operator=(const CButtonMap& rhs)
{
  if (this != &rhs)
  {
    Reset();
    for (Buttons::const_iterator it = rhs.m_buttons.begin(); it != rhs.m_buttons.end(); ++it)
      m_buttons[it->first] = it->second->Clone();
  }
  return *this;
}

void CButtonMap::Reset(void)
{
  for (Buttons::iterator it = m_buttons.begin(); it != m_buttons.end(); ++it)
    delete it->second;
  m_buttons.clear();
}

bool CButtonMap::GetFeatures(std::vector<ADDON::JoystickFeature*>& features) const
{
  for (Buttons::const_iterator itButton = m_buttons.begin(); itButton != m_buttons.end(); ++itButton)
    features.push_back(itButton->second);

  return true;
}

bool CButtonMap::MapFeature(const ADDON::JoystickFeature* feature)
{
  bool bModified = false;

  if (feature && !feature->Name().empty())
  {
    const std::string& strFeatureName = feature->Name();

    Buttons::iterator itFeature = m_buttons.find(strFeatureName);
    const bool bExists = (itFeature != m_buttons.end());

    if (bExists && itFeature->second->Equals(feature))
    {
      dsyslog("Button map: relationship for \"%s\" already exists", strFeatureName.c_str());
    }
    else
    {
      dsyslog("Button map: %s \"%s\"", bExists ? "updating" : "adding", strFeatureName.c_str());

      // If button map is modified, iterator may be invalidated
      if (UnMap(feature))
        itFeature = m_buttons.find(strFeatureName);

      if (itFeature == m_buttons.end())
      {
        m_buttons[strFeatureName] = feature->Clone();
      }
      else
      {
        delete itFeature->second;
        itFeature->second = feature->Clone();
      }

      bModified = true;
    }
  }

  return bModified;
}

bool CButtonMap::UnMap(const ADDON::JoystickFeature* feature)
{
  switch (feature->Type())
  {
    case JOYSTICK_DRIVER_TYPE_BUTTON:
      return UnMapButton(static_cast<const ADDON::DriverButton*>(feature));
    case JOYSTICK_DRIVER_TYPE_HAT_DIRECTION:
      return UnMapHat(static_cast<const ADDON::DriverHat*>(feature));
    case JOYSTICK_DRIVER_TYPE_SEMIAXIS:
      return UnMapSemiAxis(static_cast<const ADDON::DriverSemiAxis*>(feature));
    case JOYSTICK_DRIVER_TYPE_ANALOG_STICK:
      return UnMapAnalogStick(static_cast<const ADDON::DriverAnalogStick*>(feature));
    case JOYSTICK_DRIVER_TYPE_ACCELEROMETER:
      return UnMapAccelerometer(static_cast<const ADDON::DriverAccelerometer*>(feature));
    default:
      break;
  }

  return false;
}

bool CButtonMap::UnMapButton(const ADDON::DriverButton* button)
{
  for (Buttons::iterator it = m_buttons.begin(); it != m_buttons.end(); ++it)
  {
    if (ButtonConflicts(button, it->second))
    {
      dsyslog("Removing \"%s\" from button map due to conflict", it->second->Name().c_str());
      delete it->second;
      m_buttons.erase(it);
      return true;
    }
  }

  return false;
}

bool CButtonMap::UnMapHat(const ADDON::DriverHat* hat)
{
  for (Buttons::iterator it = m_buttons.begin(); it != m_buttons.end(); ++it)
  {
    if (HatConflicts(hat, it->second))
    {
      dsyslog("Removing \"%s\" from button map due to conflict", it->second->Name().c_str());
      delete it->second;
      m_buttons.erase(it);
      return true;
    }
  }

  return false;
}

bool CButtonMap::UnMapSemiAxis(const ADDON::DriverSemiAxis* semiAxis)
{
  for (Buttons::iterator it = m_buttons.begin(); it != m_buttons.end(); ++it)
  {
    if (SemiAxisConflicts(semiAxis, it->second))
    {
      switch (it->second->Type())
      {
        case JOYSTICK_DRIVER_TYPE_SEMIAXIS:
        {
          dsyslog("Removing \"%s\" from button map due to conflict", it->second->Name().c_str());
          delete it->second;
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
            {
              dsyslog("Removing \"%s\" from button map due to conflict", it->second->Name().c_str());
              delete it->second;
              m_buttons.erase(it);
            }
          }
          else if (semiAxis->Index() == analogStick->YIndex())
          {
            analogStick->SetYIndex(-1);
            if (analogStick->XIndex() < 0)
            {
              dsyslog("Removing \"%s\" from button map due to conflict", it->second->Name().c_str());
              delete it->second;
              m_buttons.erase(it);
            }
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
            {
              dsyslog("Removing \"%s\" from button map due to conflict", it->second->Name().c_str());
              delete it->second;
              m_buttons.erase(it);
            }
          }
          else if (semiAxis->Index() == accelerometer->YIndex())
          {
            accelerometer->SetYIndex(-1);
            if (accelerometer->XIndex() < 0 && accelerometer->ZIndex() < 0)
            {
              dsyslog("Removing \"%s\" from button map due to conflict", it->second->Name().c_str());
              delete it->second;
              m_buttons.erase(it);
            }
          }
          else if (semiAxis->Index() == accelerometer->ZIndex())
          {
            accelerometer->SetZIndex(-1);
            if (accelerometer->XIndex() < 0 && accelerometer->YIndex() < 0)
            {
              dsyslog("Removing \"%s\" from button map due to conflict", it->second->Name().c_str());
              delete it->second;
              m_buttons.erase(it);
            }
          }
          break;
        }
        default:
          break;
      }

      return true; // Semi-axis conflicts
    }
  }

  return false;
}

bool CButtonMap::UnMapAnalogStick(const ADDON::DriverAnalogStick* analogStick)
{
  bool bModified = false;

  ADDON::DriverSemiAxis semiAxis;

  if (analogStick->XIndex() >= 0)
  {
    semiAxis.SetIndex(analogStick->XIndex());
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE);
    bModified |= UnMapSemiAxis(&semiAxis);
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE);
    bModified |= UnMapSemiAxis(&semiAxis);
  }

  if (analogStick->YIndex() >= 0)
  {
    semiAxis.SetIndex(analogStick->YIndex());
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE);
    bModified |= UnMapSemiAxis(&semiAxis);
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE);
    bModified |= UnMapSemiAxis(&semiAxis);
  }

  return bModified;
}

bool CButtonMap::UnMapAccelerometer(const ADDON::DriverAccelerometer* accelerometer)
{
  bool bModified = false;

  ADDON::DriverSemiAxis semiAxis;

  if (accelerometer->XIndex() >= 0)
  {
    semiAxis.SetIndex(accelerometer->XIndex());
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE);
    bModified |= UnMapSemiAxis(&semiAxis);
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE);
    bModified |= UnMapSemiAxis(&semiAxis);
  }

  if (accelerometer->YIndex() >= 0)
  {
    semiAxis.SetIndex(accelerometer->YIndex());
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE);
    bModified |= UnMapSemiAxis(&semiAxis);
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE);
    bModified |= UnMapSemiAxis(&semiAxis);
  }

  if (accelerometer->ZIndex() >= 0)
  {
    semiAxis.SetIndex(accelerometer->ZIndex());
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE);
    bModified |= UnMapSemiAxis(&semiAxis);
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE);
    bModified |= UnMapSemiAxis(&semiAxis);
  }

  return bModified;
}

bool CButtonMap::ButtonConflicts(const ADDON::DriverButton* button, const ADDON::JoystickFeature* feature)
{
  if (feature->Type() == JOYSTICK_DRIVER_TYPE_BUTTON)
  {
    const ADDON::DriverButton* button2 = static_cast<const ADDON::DriverButton*>(feature);
    return button->Index() == button2->Index();
  }
  return false;
}

bool CButtonMap::HatConflicts(const ADDON::DriverHat* hat, const ADDON::JoystickFeature* feature)
{
  if (feature->Type() == JOYSTICK_DRIVER_TYPE_HAT_DIRECTION)
  {
    const ADDON::DriverHat* hat2 = static_cast<const ADDON::DriverHat*>(feature);
    return hat->Index()     == hat2->Index()  &&
           hat->Direction() == hat2->Direction();
  }
  return false;
}

bool CButtonMap::SemiAxisConflicts(const ADDON::DriverSemiAxis* semiAxis, const ADDON::JoystickFeature* feature)
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
