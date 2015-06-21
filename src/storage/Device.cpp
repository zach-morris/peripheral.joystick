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

#include "Device.h"

using namespace JOYSTICK;

CDevice::CDevice(void)
  : m_strName(),
    m_strProvider(),
    m_vid(0),
    m_pid(0),
    m_buttonCount(0),
    m_hatCount(0),
    m_axisCount(0)
{
}

CDevice::CDevice(const ADDON::Joystick& joystick)
 : m_strName(joystick.Name()),
   m_strProvider(joystick.Provider()),
   m_vid(joystick.VendorID()),
   m_pid(joystick.ProductID()),
   m_buttonCount(joystick.ButtonCount()),
   m_hatCount(joystick.HatCount()),
   m_axisCount(joystick.AxisCount())
{
}

bool CDevice::operator==(const CDevice& rhs) const
{
  return rhs.m_strName.empty()              ? true : m_strName     == rhs.m_strName   &&
         rhs.m_strProvider.empty()          ? true : m_strProvider == rhs.m_strProvider &&
         (rhs.m_vid == 0 && rhs.m_pid == 0) ? true : m_vid == rhs.m_vid && m_pid == rhs.m_pid &&
         rhs.m_buttonCount == 0 &&
         rhs.m_hatCount    == 0 &&
         rhs.m_axisCount   == 0             ? true : m_buttonCount == rhs.m_buttonCount &&
                                                     m_hatCount    == rhs.m_hatCount    &&
                                                     m_axisCount   == rhs.m_axisCount;

}

bool CDevice::IsValid(void) const
{
  return !m_strName.empty()     &&
         !m_strProvider.empty();
}

bool CDevice::GetFeatures(const std::string& strControllerId, std::vector<ADDON::JoystickFeature*>& features) const
{
  ButtonMaps::const_iterator it = m_buttonMaps.find(strControllerId);
  if (it != m_buttonMaps.end())
    return it->second.GetFeatures(features);

  return true;
}

bool CDevice::MapFeature(const std::string& strControllerId, const ADDON::JoystickFeature* feature)
{
  return m_buttonMaps[strControllerId].MapFeature(feature);
}

bool CDevice::MergeButtonMaps(const CDevice& device)
{
  const ButtonMaps& buttonMaps = device.m_buttonMaps;

  bool bModified = false;

  for (ButtonMaps::const_iterator it = buttonMaps.begin(); it != buttonMaps.end(); ++it)
  {
    const ControllerID& strControllerId = it->first;

    ButtonMaps::const_iterator it2 = m_buttonMaps.find(strControllerId);
    if (it2 == m_buttonMaps.end())
    {
      m_buttonMaps[strControllerId] = it->second;
      bModified = true;
    }
  }

  return bModified;
}
