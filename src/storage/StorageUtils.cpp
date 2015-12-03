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

#include "StorageUtils.h"
#include "filesystem/DirectoryUtils.h"
#include "log/Log.h"

#include <set>

using namespace JOYSTICK;

std::set<std::string> CStorageUtils::m_existingDirs;

bool CStorageUtils::EnsureDirectoryExists(const std::string& path)
{
  if (m_existingDirs.find(path) != m_existingDirs.end())
    return true; // Already exists

  if (!CDirectoryUtils::Exists(path))
  {
    dsyslog("Creating directory: %s", path.c_str());
    if (!CDirectoryUtils::Create(path))
    {
      esyslog("Failed to create directory!");
      return false;
    }
  }

  m_existingDirs.insert(path);

  return true;
}
