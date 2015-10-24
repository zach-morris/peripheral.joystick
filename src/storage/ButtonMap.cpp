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

#include "ButtonMap.h"
#include "log/Log.h"

using namespace JOYSTICK;

// Helper function
JOYSTICK_DRIVER_SEMIAXIS_DIRECTION operator*(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION dir, int i)
{
  return static_cast<JOYSTICK_DRIVER_SEMIAXIS_DIRECTION>(static_cast<int>(dir) * i);
}

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

void CButtonMap::GetFeatures(std::vector<ADDON::JoystickFeature*>& features) const
{
  for (Buttons::const_iterator itButton = m_buttons.begin(); itButton != m_buttons.end(); ++itButton)
    features.push_back(itButton->second);
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

      switch (feature->Type())
      {
        case JOYSTICK_FEATURE_TYPE_PRIMITIVE:
        {
          const ADDON::PrimitiveFeature* primitive = static_cast<const ADDON::PrimitiveFeature*>(feature);
          bModified = UnmapPrimitive(primitive->Primitive());
          break;
        }
        case JOYSTICK_FEATURE_TYPE_ANALOG_STICK:
        {
          const ADDON::AnalogStick* analogStick = static_cast<const ADDON::AnalogStick*>(feature);
          bModified = UnmapPrimitive(analogStick->Up()) ||
                      UnmapPrimitive(analogStick->Down()) ||
                      UnmapPrimitive(analogStick->Right()) ||
                      UnmapPrimitive(analogStick->Left());
          break;
        }
        case JOYSTICK_FEATURE_TYPE_ACCELEROMETER:
        {
          // TODO: Unmap complementary semiaxes
          const ADDON::Accelerometer* accelerometer = static_cast<const ADDON::Accelerometer*>(feature);
          bModified = UnmapPrimitive(accelerometer->PositiveX()) ||
                      UnmapPrimitive(accelerometer->PositiveY()) ||
                      UnmapPrimitive(accelerometer->PositiveZ());
          break;
        }
        default:
          break;
      }

      // If button map is modified, iterator may be invalidated
      if (bModified)
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

bool CButtonMap::UnmapPrimitive(const ADDON::DriverPrimitive& primitive)
{
  bool bModified = false;

  for (Buttons::iterator it = m_buttons.begin(); it != m_buttons.end(); ++it)
  {
    ADDON::JoystickFeature* feature = it->second;
    switch (feature->Type())
    {
      case JOYSTICK_FEATURE_TYPE_PRIMITIVE:
      {
        ADDON::PrimitiveFeature* primitiveFeature = static_cast<ADDON::PrimitiveFeature*>(feature);
        if (primitive.Equals(primitiveFeature->Primitive()))
        {
          dsyslog("Removing \"%s\" from button map due to conflict", feature->Name().c_str());
          delete feature;
          m_buttons.erase(it);
          bModified = true;
        }

        break;
      }
      case JOYSTICK_FEATURE_TYPE_ANALOG_STICK:
      {
        ADDON::AnalogStick* analogStick = static_cast<ADDON::AnalogStick*>(feature);
        if (primitive.Equals(analogStick->Up()))
        {
          analogStick->SetUp(ADDON::DriverPrimitive());
          bModified = true;
        }
        else if (primitive.Equals(analogStick->Down()))
        {
          analogStick->SetDown(ADDON::DriverPrimitive());
          bModified = true;
        }
        else if (primitive.Equals(analogStick->Right()))
        {
          analogStick->SetRight(ADDON::DriverPrimitive());
          bModified = true;
        }
        else if (primitive.Equals(analogStick->Left()))
        {
          analogStick->SetLeft(ADDON::DriverPrimitive());
          bModified = true;
        }

        if (bModified)
        {
          if (analogStick->Up().Type() == JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN &&
              analogStick->Down().Type() == JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN &&
              analogStick->Right().Type() == JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN &&
              analogStick->Left().Type() == JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN)
          {
            dsyslog("Removing \"%s\" from button map due to conflict", feature->Name().c_str());
            delete feature;
            m_buttons.erase(it);
          }
        }

        break;
      }
      case JOYSTICK_FEATURE_TYPE_ACCELEROMETER:
      {
        ADDON::Accelerometer* accelerometer = static_cast<ADDON::Accelerometer*>(feature);
        if (primitive.Equals(accelerometer->PositiveX()) ||
            primitive.Equals(Opposite(accelerometer->PositiveX())))
        {
          accelerometer->SetPositiveX(ADDON::DriverPrimitive());
          bModified = true;
        }
        else if (primitive.Equals(accelerometer->PositiveY()) ||
                 primitive.Equals(Opposite(accelerometer->PositiveY())))
        {
          accelerometer->SetPositiveY(ADDON::DriverPrimitive());
          bModified = true;
        }
        else if (primitive.Equals(accelerometer->PositiveZ()) ||
                 primitive.Equals(Opposite(accelerometer->PositiveZ())))
        {
          accelerometer->SetPositiveZ(ADDON::DriverPrimitive());
          bModified = true;
        }

        if (bModified)
        {
          if (accelerometer->PositiveX().Type() == JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN &&
              accelerometer->PositiveY().Type() == JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN &&
              accelerometer->PositiveZ().Type() == JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN)
          {
            dsyslog("Removing \"%s\" from button map due to conflict", feature->Name().c_str());
            delete feature;
            m_buttons.erase(it);
          }
        }

        break;
      }
      default:
        break;
    }

    if (bModified)
      break;
  }

  return bModified;
}

ADDON::DriverPrimitive CButtonMap::Opposite(const ADDON::DriverPrimitive& primitive)
{
  return ADDON::DriverPrimitive(primitive.DriverIndex(), primitive.SemiAxisDirection() * -1);
}
