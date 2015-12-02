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

using namespace JOYSTICK;
using namespace PLATFORM;

CDatabase::CDatabase(void)
  : m_driverDatabase(nullptr),
    m_bEnabled(true)
{
}

bool CDatabase::Enable(void)
{
  if (m_driverDatabase)
  {
    m_bEnabled = true;
    return true;
  }
  return false;
}

void CDatabase::Disable(void)
{
  m_bEnabled = false;
}

bool CDatabase::GetFeatures(const CDevice& driverInfo, const std::string& controllerId,
                            FeatureVector& features)
{
  CLockObject lock(m_mutex);

  Records::const_iterator itDevice = m_records.find(driverInfo);
  if (itDevice != m_records.end())
  {
    const ButtonMaps& buttonMaps = itDevice->second;
    ButtonMaps::const_iterator itButtonMap = buttonMaps.find(controllerId);
    if (itButtonMap != buttonMaps.end())
    {
      features = itButtonMap->second;
      return true;
    }
  }

  return false;
}

bool CDatabase::MapFeatures(const CDevice& driverInfo, const std::string& controllerId,
                            const FeatureVector& features)
{
  CLockObject lock(m_mutex);

  ButtonMaps& buttonMaps = m_records[driverInfo];

  buttonMaps[controllerId] = features;

  return true;
}
