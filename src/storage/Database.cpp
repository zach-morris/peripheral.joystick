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

#include "Database.h"

#include <algorithm>

using namespace JOYSTICK;

bool CDatabase::GetFeatures(const CDevice& needle, const std::string& strControllerId,
                            std::vector<ADDON::JoystickFeature*>& features)
{
  std::vector<CDevice>::const_iterator itDevice = std::find(m_devices.begin(), m_devices.end(), needle);
  if (itDevice != m_devices.end())
    return itDevice->GetFeatures(strControllerId, features);

  return false;
}

bool CDatabase::MapFeature(const CDevice& needle, const std::string& strControllerId,
                           const ADDON::JoystickFeature* feature)
{
  std::vector<CDevice>::iterator itDevice = std::find(m_devices.begin(), m_devices.end(), needle);

  // Create a new object if device wasn't found
  if (itDevice == m_devices.end())
  {
    m_devices.push_back(needle);
    itDevice = m_devices.end() - 1;
  }

  return itDevice->MapFeature(strControllerId, feature);
}
