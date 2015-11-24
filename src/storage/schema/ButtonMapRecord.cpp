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

#include "ButtonMapRecord.h"
#include "log/Log.h"

using namespace JOYSTICK;

// Helper function
JOYSTICK_DRIVER_SEMIAXIS_DIRECTION operator*(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION dir, int i)
{
  return static_cast<JOYSTICK_DRIVER_SEMIAXIS_DIRECTION>(static_cast<int>(dir) * i);
}

CButtonMapRecord::CButtonMapRecord(const ADDON::Joystick& driverInfo, const std::string& controllerId)
 : m_driverProperties(driverInfo),
   m_controllerId(controllerId)
{
}

CButtonMapRecord::~CButtonMapRecord(void)
{
  for (ButtonMap::iterator it = m_buttonMap.begin(); it != m_buttonMap.end(); ++it)
    delete it->second;
}

CButtonMapRecord& CButtonMapRecord::operator=(CButtonMapRecord&& rhs)
{
  if (this != &rhs)
  {
    m_driverProperties = rhs.m_driverProperties; // TODO: possible optimization here
    m_controllerId = std::move(rhs.m_controllerId);
    m_buttonMap = std::move(rhs.m_buttonMap);
  }
  return *this;
}

void CButtonMapRecord::GetFeatures(FeatureVector& features) const
{
  for (ButtonMap::const_iterator itButton = m_buttonMap.begin(); itButton != m_buttonMap.end(); ++itButton)
    features.push_back(FeaturePtr(itButton->second->Clone()));
}

bool CButtonMapRecord::MapFeature(const ADDON::JoystickFeature* feature)
{
  bool bModified = false;

  if (feature && !feature->Name().empty())
  {
    const std::string& strFeatureName = feature->Name();

    ButtonMap::iterator itFeature = m_buttonMap.find(strFeatureName);
    const bool bExists = (itFeature != m_buttonMap.end());

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
        itFeature = m_buttonMap.find(strFeatureName);

      if (itFeature == m_buttonMap.end())
      {
        m_buttonMap[strFeatureName] = feature->Clone();
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

bool CButtonMapRecord::UnmapPrimitive(const ADDON::DriverPrimitive& primitive)
{
  bool bModified = false;

  for (ButtonMap::iterator it = m_buttonMap.begin(); it != m_buttonMap.end(); ++it)
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
          m_buttonMap.erase(it);
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
            m_buttonMap.erase(it);
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
            m_buttonMap.erase(it);
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

ADDON::DriverPrimitive CButtonMapRecord::Opposite(const ADDON::DriverPrimitive& primitive)
{
  return ADDON::DriverPrimitive(primitive.DriverIndex(), primitive.SemiAxisDirection() * -1);
}
