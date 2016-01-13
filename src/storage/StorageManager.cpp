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

#include "StorageManager.h"
#include "JustABunchOfFiles.h"
#include "StorageUtils.h"
#include "log/Log.h"
//#include "storage/retroarch/DatabaseRetroarch.h" // TODO
#include "storage/xml/DatabaseXml.h"
#include "utils/StringUtils.h"

#include "kodi/libKODI_peripheral.h"

using namespace JOYSTICK;

// Resources folder for add-on and user data
#define USER_RESOURCES_FOLDER   "resources"
#define ADDON_RESOURCES_FOLDER  "resources"

// Subdirectory under resources folder for storing button maps
#define BUTTONMAP_FOLDER        "buttonmaps"

CStorageManager::CStorageManager(void) :
  m_peripheralLib(NULL)
{
}

CStorageManager& CStorageManager::Get(void)
{
  static CStorageManager _instance;
  return _instance;
}

bool CStorageManager::Initialize(ADDON::CHelper_libKODI_peripheral* peripheralLib,
                                 const PERIPHERAL_PROPERTIES& props)
{
  std::string strUserPath = props.user_path ? props.user_path : "";
  std::string strAddonPath = props.addon_path ? props.addon_path : "";

  if (peripheralLib == NULL || strUserPath.empty() || strAddonPath.empty())
    return false;

  m_peripheralLib = peripheralLib;

  // Remove slash at end
  StringUtils::TrimRight(strUserPath, "\\/");
  StringUtils::TrimRight(strAddonPath, "\\/");

  strUserPath += "/" USER_RESOURCES_FOLDER;
  strAddonPath += "/" ADDON_RESOURCES_FOLDER;

  // Ensure resources path exists in user data
  CStorageUtils::EnsureDirectoryExists(strUserPath);

  strUserPath += "/" BUTTONMAP_FOLDER;
  strAddonPath += "/" BUTTONMAP_FOLDER;

  // Ensure button map path exists in user data
  CStorageUtils::EnsureDirectoryExists(strUserPath);

  m_databases.push_back(DatabasePtr(new CDatabaseXml(strUserPath, true)));
  //m_databases.push_back(DatabasePtr(new CDatabaseRetroArch(strUserPath, true))); // TODO
  m_databases.push_back(DatabasePtr(new CDatabaseXml(strAddonPath, false)));
  //m_databases.push_back(DatabasePtr(new CDatabaseRetroArch(strAddonPath, false))); // TODO

  return true;
}

void CStorageManager::Deinitialize(void)
{
  m_peripheralLib = NULL;
}

bool CStorageManager::GetFeatures(const ADDON::Joystick& joystick,
                                  const std::string& strControllerId,
                                  FeatureVector& features)
{
  CDevice deviceInfo(joystick);

  for (DatabaseVector::const_iterator it = m_databases.begin(); it != m_databases.end(); ++it)
  {
    if ((*it)->GetFeatures(deviceInfo, strControllerId, features))
      return true;
  }

  return true;
}

bool CStorageManager::MapFeatures(const ADDON::Joystick& joystick,
                                  const std::string& strControllerId,
                                  const FeatureVector& features)
{
  CDevice deviceInfo(joystick);

  bool bModified = false;

  for (DatabaseVector::const_iterator it = m_databases.begin(); it != m_databases.end(); ++it)
    bModified |= (*it)->MapFeatures(deviceInfo, strControllerId, features);

  if (bModified)
    RefreshButtonMaps(deviceInfo.Name(), strControllerId);

  return bModified;
}

void CStorageManager::ResetButtonMap(const ADDON::Joystick& joystick, const std::string& strControllerId)
{
  CDevice deviceInfo(joystick);

  bool bModified = false;

  for (DatabaseVector::const_iterator it = m_databases.begin(); it != m_databases.end(); ++it)
    bModified |= (*it)->ResetButtonMap(deviceInfo, strControllerId);

  if (bModified)
    RefreshButtonMaps(deviceInfo.Name(), strControllerId);
}

void CStorageManager::RefreshButtonMaps(const std::string& strDeviceName /* = "" */,
                                        const std::string& strControllerId /* = "" */)
{
  // Request the frontend to refresh its button maps
  if (m_peripheralLib)
    m_peripheralLib->RefreshButtonMaps(strDeviceName, strControllerId);
}
