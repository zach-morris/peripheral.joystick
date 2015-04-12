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

#include "Devices.h"

#include <algorithm>

using namespace JOYSTICK;

CDevices& CDevices::Get(void)
{
  static CDevices _instance;
  return _instance;
}

bool CDevices::GetFeatures(const ADDON::Peripheral& peripheral, const ADDON::Joystick& joystick,
                           const std::string& strDeviceId, std::vector<ADDON::JoystickFeature*>& features) const
{
  const std::string& strName     = peripheral.Name();
  uint16_t           vid         = peripheral.VendorID();
  uint16_t           pid         = peripheral.ProductID();
  const std::string& strProvider = joystick.Provider();
  unsigned int       buttonCount = joystick.ButtonCount();
  unsigned int       hatCount    = joystick.HatCount();
  unsigned int       axisCount   = joystick.AxisCount();

  CDevice needle(strName,
                 strProvider,
                 vid,
                 pid,
                 buttonCount,
                 hatCount,
                 axisCount);

  std::vector<CDevice>::const_iterator itDevice = std::find(m_devices.begin(), m_devices.end(), needle);
  if (itDevice != m_devices.end())
    return itDevice->GetFeatures(strDeviceId, features);

  return true;
}

bool CDevices::MapFeature(const ADDON::Peripheral& peripheral, const ADDON::Joystick& joystick,
                          const std::string& strDeviceId, const ADDON::JoystickFeature* feature)
{
  const std::string& strName     = peripheral.Name();
  uint16_t           vid         = peripheral.VendorID();
  uint16_t           pid         = peripheral.ProductID();
  const std::string& strProvider = joystick.Provider();
  unsigned int       buttonCount = joystick.ButtonCount();
  unsigned int       hatCount    = joystick.HatCount();
  unsigned int       axisCount   = joystick.AxisCount();

  CDevice needle(strName,
                 strProvider,
                 vid,
                 pid,
                 buttonCount,
                 hatCount,
                 axisCount);

  std::vector<CDevice>::iterator itDevice = std::find(m_devices.begin(), m_devices.end(), needle);
  if (itDevice == m_devices.end())
  {
    m_devices.push_back(needle);
    itDevice = m_devices.end() - 1;
  }

  return itDevice->MapFeature(strDeviceId, feature);
}
