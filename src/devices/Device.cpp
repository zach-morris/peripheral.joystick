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

CDevice::CDevice(const std::string& strName,
                 const std::string& strProvider,
                 uint16_t           vid,
                 uint16_t           pid,
                 unsigned int       buttonCount,
                 unsigned int       hatCount,
                 unsigned int       axisCount)
 : m_strName(strName),
   m_strProvider(strProvider),
   m_pid(pid),
   m_vid(vid),
   m_buttonCount(buttonCount),
   m_hatCount(hatCount),
   m_axisCount(axisCount)
{
}

bool CDevice::operator==(const CDevice& rhs) const
{
  return rhs.m_strName.empty()              ? true : m_strName     == rhs.m_strName   &&
         rhs.m_strProvider.empty()          ? true : m_strProvider == rhs.m_strProvider &&
         (rhs.m_pid == 0 && rhs.m_vid == 0) ? true : m_pid == rhs.m_pid && m_vid == rhs.m_vid &&
         rhs.m_buttonCount == 0 &&
         rhs.m_hatCount    == 0 &&
         rhs.m_axisCount   == 0             ? true : m_buttonCount == rhs.m_buttonCount &&
                                                     m_hatCount    == rhs.m_hatCount    &&
                                                     m_axisCount   == rhs.m_axisCount;

}

bool CDevice::GetFeatures(const std::string& strDeviceId, std::vector<ADDON::JoystickFeature*>& features) const
{
  ButtonMaps::const_iterator it = m_buttonMaps.find(strDeviceId);
  if (it != m_buttonMaps.end())
    return it->second.GetFeatures(features);

  return true;
}

bool CDevice::MapFeature(const std::string& strDeviceId, const ADDON::JoystickFeature* feature)
{
  return m_buttonMaps[strDeviceId].MapFeature(feature);
}
