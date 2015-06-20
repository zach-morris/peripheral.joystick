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

#include "StorageManager.h"
#include "storage/web/DatabaseWeb.h"
#include "storage/xml/DatabaseXml.h"

using namespace JOYSTICK;

#define ADDON_BUTTONMAP  "/resources/buttonmap.xml"
#define USER_BUTTONMAP   "/buttonmap.xml"

// --- RemoveSlashAtEnd --------------------------------------------------------

namespace JOYSTICK
{
  void RemoveSlashAtEnd(std::string& strPath)
  {
    if (strPath.empty())
    {
      char end = strPath[strPath.size() - 1];
      if (end == '/' || end == '\\')
        strPath.erase(strPath.end() - 1);
    }
  }
}

// --- CStorageManager ---------------------------------------------------------

CStorageManager& CStorageManager::Get(void)
{
  static CStorageManager _instance;
  return _instance;
}

bool CStorageManager::Initialize(const PERIPHERAL_PROPERTIES& props)
{
  std::string strAddonPath = props.addon_path ? props.addon_path : "";
  std::string strUserPath = props.user_path ? props.user_path : "";

  if (strAddonPath.empty() || strUserPath.empty())
    return false;

  RemoveSlashAtEnd(strAddonPath);
  RemoveSlashAtEnd(strUserPath);

  strAddonPath += ADDON_BUTTONMAP;
  strUserPath += USER_BUTTONMAP;

  CDatabase* userXml = new CDatabaseXml(strUserPath, false);
  CDatabase* addonXml = new CDatabaseXml(strAddonPath, true);

  m_databases.push_back(userXml);
  m_databases.push_back(new CDatabaseWeb(userXml));
  m_databases.push_back(addonXml);

  return true;
}

void CStorageManager::Deinitialize(void)
{
  for (std::vector<CDatabase*>::iterator it = m_databases.begin(); it != m_databases.end(); ++it)
    delete *it;
}

bool CStorageManager::GetFeatures(const ADDON::Joystick& joystick, const std::string& strControllerId,
                                  std::vector<ADDON::JoystickFeature*>& features)
{
  CDevice needle(joystick);

  for (std::vector<CDatabase*>::const_iterator it = m_databases.begin(); it != m_databases.end(); ++it)
  {
    if ((*it)->GetFeatures(needle, strControllerId, features))
      return true;
  }

  return true;
}

bool CStorageManager::MapFeature(const ADDON::Joystick& joystick, const std::string& strControllerId,
                                 const ADDON::JoystickFeature* feature)
{
  CDevice needle(joystick);

  bool bSuccess = false;

  for (std::vector<CDatabase*>::const_iterator it = m_databases.begin(); it != m_databases.end(); ++it)
    bSuccess |= (*it)->MapFeature(needle, strControllerId, feature);

  return bSuccess;
}
