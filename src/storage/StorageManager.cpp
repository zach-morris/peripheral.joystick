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
#include "storage/generic/DeviceDatabase.h" // TODO
#include "filesystem/DirectoryUtils.h"
#include "log/Log.h"
#include "storage/xml/DatabaseXml.h"

#include "kodi/libKODI_peripheral.h"

using namespace JOYSTICK;

// Resources subfolder for add-on and user data
#define USER_RESOURCES_FOLDER   "resources"
#define ADDON_RESOURCES_FOLDER  "resources"

// --- RemoveSlashAtEnd --------------------------------------------------------

namespace JOYSTICK
{
  bool IsSlash(char c)
  {
    return c == '/' || c == '\\';
  }

  void RemoveSlashAtEnd(std::string& strPath)
  {
    if (!strPath.empty())
    {
      char end = strPath[strPath.size() - 1];
      if (IsSlash(end))
        strPath.erase(strPath.end() - 1);
    }
  }
}

// --- CStorageManager ---------------------------------------------------------

CStorageManager::CStorageManager(void)
  : m_peripheralLib(NULL)
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

  RemoveSlashAtEnd(strUserPath);
  RemoveSlashAtEnd(strAddonPath);

  strUserPath += "/" USER_RESOURCES_FOLDER;
  strAddonPath += "/" ADDON_RESOURCES_FOLDER;

  // Ensure resources path exists in user data
  if (!CDirectoryUtils::Exists(strUserPath))
  {
    dsyslog("Creating directory %s", strUserPath.c_str());
    if (!CDirectoryUtils::Create(strUserPath))
      esyslog("Failed to create directory!");
  }

  CDatabase* userDatabase = new CDatabaseXml(strUserPath, false);
  CDatabase* addonDatabase = new CDatabaseXml(strAddonPath, true);

  m_databases.push_back(userDatabase);
  m_databases.push_back(addonDatabase);

  CDriverDatabase* m_driverDatabase;

  return true;
}

void CStorageManager::Deinitialize(void)
{
  // Delete in reverse order
  for (std::vector<CDatabase*>::iterator it = m_databases.begin(); it != m_databases.end(); ++it)
    delete *it;

  m_peripheralLib = NULL;
}

bool CStorageManager::GetFeatures(const ADDON::Joystick& joystick, const std::string& strControllerId,
                                  FeatureVector& features)
{
  for (std::vector<CDatabase*>::const_iterator it = m_databases.begin(); it != m_databases.end(); ++it)
  {
    CDevice driverInfo;
    if ((*it)->m_driverDatabase->GetDevice(joystick, driverInfo)) // TODO
    {
      if ((*it)->IsEnabled() && (*it)->GetFeatures(joystick, strControllerId, features))
        break;
    }
  }

  return true;
}

bool CStorageManager::MapFeature(const ADDON::Joystick& joystick, const std::string& strControllerId,
                                 const FeaturePtr& feature)
{
  for (std::vector<CDatabase*>::const_iterator it = m_databases.begin(); it != m_databases.end(); ++it)
    (*it)->MapFeature(joystick, strControllerId, feature);

  return true;
}

void CStorageManager::RefreshButtonMaps(const std::string& strDeviceName /* = "" */)
{
  // Request the frontend to refresh its button maps
  if (m_peripheralLib)
    m_peripheralLib->RefreshButtonMaps(strDeviceName);
}
