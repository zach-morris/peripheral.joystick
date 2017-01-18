/*
 *      Copyright (C) 2016 Garrett Brown
 *      Copyright (C) 2016 Team Kodi
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
 *  along with this Program; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "DeviceConfiguration.h"
#include "Device.h"
#include "api/Joystick.h"
#include "api/JoystickManager.h"

#include <algorithm>
#include <assert.h>

using namespace JOYSTICK;

void CDeviceConfiguration::Reset(void)
{
  m_axes.clear();
  m_buttons.clear();
}

bool CDeviceConfiguration::IsEmpty() const
{
  return m_axes.empty() &&
         m_buttons.empty();
}

const AxisConfiguration& CDeviceConfiguration::Axis(unsigned int index) const
{
  static AxisConfiguration defaultConfig{ };

  auto it = m_axes.find(index);
  if (it != m_axes.end())
    return it->second;

  return defaultConfig;
}

const ButtonConfiguration& CDeviceConfiguration::Button(unsigned int index) const
{
  static ButtonConfiguration defaultConfig{ };

  auto it = m_buttons.find(index);
  if (it != m_buttons.end())
    return it->second;

  return defaultConfig;
}

void CDeviceConfiguration::LoadAxisFromAPI(unsigned int axisIndex, const CDevice& joystickInfo)
{
  // TODO
  int triggerCenter = 0;
  int triggerRange = 1;

  JoystickVector joysticks = CJoystickManager::Get().GetJoysticks(joystickInfo);
  for (const auto& joystick : joysticks)
  {
    m_axes[axisIndex].trigger.center = triggerCenter;
    m_axes[axisIndex].trigger.range = triggerRange;
  }
}

PrimitiveVector CDeviceConfiguration::GetIgnoredPrimitives() const
{
  PrimitiveVector primitives;

  for (const auto& axisConfig : m_axes)
  {
    if (axisConfig.second.bIgnore)
    {
      primitives.emplace_back(axisConfig.first, JOYSTICK_DRIVER_SEMIAXIS_POSITIVE);
      primitives.emplace_back(axisConfig.first, JOYSTICK_DRIVER_SEMIAXIS_NEGATIVE);
    }
  }

  for (const auto& buttonConfig : m_buttons)
  {
    if (buttonConfig.second.bIgnore)
      primitives.emplace_back(ADDON::DriverPrimitive::CreateButton(buttonConfig.first));
  }

  return primitives;
}

void CDeviceConfiguration::SetIgnoredPrimitives(const PrimitiveVector& primitives)
{
  // Reset known axes
  for (auto& axisConfig : m_axes)
    axisConfig.second.bIgnore = false;

  for (auto& buttonConfig : m_buttons)
    buttonConfig.second.bIgnore = false;

  // Update ignore status
  for (const auto& primitive : primitives)
  {
    switch (primitive.Type())
    {
    case JOYSTICK_DRIVER_PRIMITIVE_TYPE_BUTTON:
      m_buttons[primitive.DriverIndex()].bIgnore = true;
      break;
    case JOYSTICK_DRIVER_PRIMITIVE_TYPE_SEMIAXIS:
      m_axes[primitive.DriverIndex()].bIgnore = true;
      break;
    default:
      break;
    }
  }
}
