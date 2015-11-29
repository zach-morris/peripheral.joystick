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

#include "ButtonMap.h"
#include "log/Log.h"

#include <algorithm>

using namespace JOYSTICK;
using namespace PLATFORM;

// Helper function
JOYSTICK_DRIVER_SEMIAXIS_DIRECTION operator*(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION dir, int i)
{
  return static_cast<JOYSTICK_DRIVER_SEMIAXIS_DIRECTION>(static_cast<int>(dir) * i);
}

CButtonMap& CButtonMap::operator=(CButtonMap&& rhs)
{
  if (this != &rhs)
  {
    m_buttonMap = std::move(rhs.m_buttonMap);
  }
  return *this;
}

bool CButtonMap::MapFeature(const ADDON::JoystickFeature& feature)
{
  bool bModified = false; // Return value

  if (feature.Name().empty())
    return bModified;

  const std::string& strFeatureName = feature.Name();

  // Look up existing feature by name
  FeatureVector::iterator itFeature = std::find_if(m_buttonMap.begin(), m_buttonMap.end(),
    [&strFeatureName](const ADDON::JoystickFeature& existingFeature)
    {
      return existingFeature.Name() == strFeatureName;
    });

  // Calculate properties of new feature
  bool bExists      = (itFeature != m_buttonMap.end());
  bool bIsValid     = (feature.Type() != JOYSTICK_FEATURE_TYPE_UNKNOWN);
  bool bIsUnchanged = false;

  if (bExists)
  {
    const ADDON::JoystickFeature& existingFeature = *itFeature;
    bIsUnchanged = (existingFeature == feature);
  }

  // Process changes
  if (!bExists)
  {
    dsyslog("Button map: adding new feature \"%s\"", strFeatureName.c_str());
    m_buttonMap.push_back(feature);
    bModified = true;
  }
  else if (!bIsValid)
  {
    dsyslog("Button map: removing \"%s\"", strFeatureName.c_str());
    m_buttonMap.erase(itFeature);
    bModified = true;
  }
  else if (bIsUnchanged)
  {
    dsyslog("Button map: relationship for \"%s\" unchanged", strFeatureName.c_str());
  }
  else
  {
    bool bUnmapped = UnmapFeature(feature);

    *itFeature = feature;

    // UnmapFeature() may have invalidated a feature in the button map
    if (bUnmapped)
    {
      m_buttonMap.erase(std::remove_if(m_buttonMap.begin(), m_buttonMap.end(),
        [](const ADDON::JoystickFeature& feature)
        {
          return feature.Type() == JOYSTICK_FEATURE_TYPE_UNKNOWN;
        }), m_buttonMap.end());
    }

    bModified = true;
  }

  return bModified;
}

bool CButtonMap::UnmapFeature(const ADDON::JoystickFeature& feature)
{
  bool bModified = false;

  switch (feature.Type())
  {
    case JOYSTICK_FEATURE_TYPE_SCALAR:
    {
      bModified = UnmapPrimitive(feature.Primitive());
      break;
    }
    case JOYSTICK_FEATURE_TYPE_ANALOG_STICK:
    {
      bModified = UnmapPrimitive(feature.Up()) ||
                  UnmapPrimitive(feature.Down()) ||
                  UnmapPrimitive(feature.Right()) ||
                  UnmapPrimitive(feature.Left());
      break;
    }
    case JOYSTICK_FEATURE_TYPE_ACCELEROMETER:
    {
      // TODO: Unmap complementary semiaxes
      bModified = UnmapPrimitive(feature.PositiveX()) ||
                  UnmapPrimitive(feature.PositiveY()) ||
                  UnmapPrimitive(feature.PositiveZ());
      break;
    }
    default:
      break;
  }

  return bModified;
}

bool CButtonMap::UnmapPrimitive(const ADDON::DriverPrimitive& primitive)
{
  bool bModified = false;

  if (primitive.Type() == JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN)
    return bModified;

  for (FeatureVector::iterator it = m_buttonMap.begin(); it != m_buttonMap.end(); ++it)
  {
    ADDON::JoystickFeature& feature = *it;
    switch (feature.Type())
    {
      case JOYSTICK_FEATURE_TYPE_SCALAR:
      {
        if (primitive == feature.Primitive())
        {
          dsyslog("Removing \"%s\" from button map due to conflict", feature.Name().c_str());
          it->SetType(JOYSTICK_FEATURE_TYPE_UNKNOWN);
          bModified = true;
        }

        break;
      }
      case JOYSTICK_FEATURE_TYPE_ANALOG_STICK:
      {
        if (primitive == feature.Up())
        {
          feature.SetUp(ADDON::DriverPrimitive());
          bModified = true;
        }
        else if (primitive == feature.Down())
        {
          feature.SetDown(ADDON::DriverPrimitive());
          bModified = true;
        }
        else if (primitive == feature.Right())
        {
          feature.SetRight(ADDON::DriverPrimitive());
          bModified = true;
        }
        else if (primitive == feature.Left())
        {
          feature.SetLeft(ADDON::DriverPrimitive());
          bModified = true;
        }

        if (bModified)
        {
          if (feature.Up().Type() == JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN &&
              feature.Down().Type() == JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN &&
              feature.Right().Type() == JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN &&
              feature.Left().Type() == JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN)
          {
            dsyslog("Removing \"%s\" from button map due to conflict", feature.Name().c_str());
            it->SetType(JOYSTICK_FEATURE_TYPE_UNKNOWN);
          }
        }

        break;
      }
      case JOYSTICK_FEATURE_TYPE_ACCELEROMETER:
      {
        if (primitive == feature.PositiveX() ||
            primitive == Opposite(feature.PositiveX()))
        {
          feature.SetPositiveX(ADDON::DriverPrimitive());
          bModified = true;
        }
        else if (primitive == feature.PositiveY() ||
                 primitive == Opposite(feature.PositiveY()))
        {
          feature.SetPositiveY(ADDON::DriverPrimitive());
          bModified = true;
        }
        else if (primitive == feature.PositiveZ() ||
                 primitive == Opposite(feature.PositiveZ()))
        {
          feature.SetPositiveZ(ADDON::DriverPrimitive());
          bModified = true;
        }

        if (bModified)
        {
          if (feature.PositiveX().Type() == JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN &&
              feature.PositiveY().Type() == JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN &&
              feature.PositiveZ().Type() == JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN)
          {
            dsyslog("Removing \"%s\" from button map due to conflict", feature.Name().c_str());
            it->SetType(JOYSTICK_FEATURE_TYPE_UNKNOWN);
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
