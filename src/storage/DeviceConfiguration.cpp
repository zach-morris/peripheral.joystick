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
#include "api/AnomalousTrigger.h"
#include "api/Joystick.h"
#include "api/JoystickManager.h"

#include <algorithm>
#include <assert.h>

using namespace JOYSTICK;

CDeviceConfiguration::CDeviceConfiguration(const CDevice* parent) :
  m_parent(parent)
{
  assert(m_parent != nullptr);
}

void CDeviceConfiguration::Reset(void)
{
  m_axes.clear();
}

bool CDeviceConfiguration::GetAxis(unsigned int axisIndex, AxisProperties& axisProps) const
{
  auto itAxis = m_axes.find(axisIndex);
  if (itAxis != m_axes.end())
  {
    axisProps = itAxis->second;
    return true;
  }

  return false;
}

void CDeviceConfiguration::LoadAxis(unsigned int axisIndex)
{
  JoystickVector joysticks = CJoystickManager::Get().GetJoysticks(*m_parent);
  for (const auto& joystick : joysticks)
  {
    std::vector<CAnomalousTrigger*> triggerVec = joystick->GetAnomalousTriggers();

    auto itTrigger = std::find_if(triggerVec.begin(), triggerVec.end(),
      [axisIndex](const CAnomalousTrigger* trigger)
      {
        return trigger->AxisIndex() == axisIndex;
      });

    if (itTrigger != triggerVec.end())
    {
      m_axes[axisIndex] = { axisIndex, (*itTrigger)->Center(), (*itTrigger)->Range() };
    }
    else
    {
      auto itAxis = m_axes.find(axisIndex);
      if (itAxis != m_axes.end())
        m_axes.erase(itAxis);
    }
  }
}

void CDeviceConfiguration::SetAxis(const AxisProperties& axisProps)
{
  m_axes[axisProps.index] = axisProps;
}
