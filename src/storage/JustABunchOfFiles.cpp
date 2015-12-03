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

#include "JustABunchOfFiles.h"
#include "ButtonMapDefinitions.h"
#include "StorageUtils.h"
#include "filesystem/DirectoryUtils.h"
#include "log/Log.h"
#include "utils/StringUtils.h"

#include <algorithm>
#include <assert.h>

using namespace JOYSTICK;
using namespace PLATFORM;

#define FOLDER_DEPTH  1  // Recurse into max 1 subdirectories (provider)

// --- CResources --------------------------------------------------------------

CResources::~CResources(void)
{
  for (ResourceMap::iterator it = m_resources.begin(); it != m_resources.end(); ++it)
    delete it->second;
}

CButtonMap* CResources::GetResource(const CDevice& deviceInfo)
{
  ResourceMap::iterator itResource = m_resources.find(deviceInfo);
  if (itResource != m_resources.end())
    return itResource->second;

  return nullptr;
}

void CResources::AddResource(CButtonMap* resource)
{
  assert(resource != nullptr);

  if (resource->Device().IsValid())
    m_resources[resource->Device()] = resource;
}

void CResources::RemoveResource(const std::string& strPath)
{
  for (ResourceMap::iterator it = m_resources.begin(); it != m_resources.end(); ++it)
  {
    if (it->second->Path() == strPath)
    {
      delete it->second;
      m_resources.erase(it);
      break;
    }
  }
}

// --- CJustABunchOfFiles ------------------------------------------------------

CJustABunchOfFiles::CJustABunchOfFiles(const std::string& strResourcePath, const std::string& strExtension, bool bReadWrite) :
  m_strResourcePath(strResourcePath),
  m_strExtension(strExtension),
  m_bReadWrite(bReadWrite)
{
  m_directoryCache.Initialize(this);

  if (m_bReadWrite)
    CStorageUtils::EnsureDirectoryExists(m_strResourcePath);
}

CJustABunchOfFiles::~CJustABunchOfFiles(void)
{
  m_directoryCache.Deinitialize();
}

bool CJustABunchOfFiles::GetFeatures(const CDevice& driverInfo,
                                     const std::string& controllerId,
                                     FeatureVector& features)
{
  CLockObject lock(m_mutex);

  // Update index
  IndexDirectory(m_strResourcePath, FOLDER_DEPTH);

  CButtonMap* resource = m_resources.GetResource(driverInfo);

  if (resource)
    return resource->GetFeatures(controllerId, features);

  return false;
}

bool CJustABunchOfFiles::MapFeatures(const CDevice& driverInfo,
                                     const std::string& controllerId,
                                     const FeatureVector& features)
{
  if (!m_bReadWrite)
    return false;

  CLockObject lock(m_mutex);

  CButtonMap* resource = m_resources.GetResource(driverInfo);

  if (resource == nullptr)
  {
    // Resource doesn't exist yet, try to create it now
    std::string resourcePath;
    if (GetResourcePath(driverInfo, resourcePath))
    {
      resource = CreateResource(resourcePath, driverInfo);
      m_resources.AddResource(resource);
    }
  }

  if (resource)
    return resource->MapFeatures(controllerId, features);

  return false;
}

void CJustABunchOfFiles::IndexDirectory(const std::string& path, unsigned int folderDepth)
{
  std::vector<ADDON::CVFSDirEntry> items;
  if (!m_directoryCache.GetDirectory(path, items))
    CDirectoryUtils::GetDirectory(path, m_strExtension + "|", items);

  // Recurse into subdirectories
  if (folderDepth > 0)
  {
    for (std::vector<ADDON::CVFSDirEntry>::const_iterator it = items.begin(); it != items.end(); ++it)
    {
      const ADDON::CVFSDirEntry& item = *it;
      if (item.IsFolder())
      {
        IndexDirectory(item.Path(), folderDepth - 1);
      }
    }
  }

  // Erase all folders and resources with different extensions
  items.erase(std::remove_if(items.begin(), items.end(),
    [this](const ADDON::CVFSDirEntry& item)
    {
      return !item.IsFolder() && !StringUtils::EndsWith(item.Path(), this->m_strExtension);
    }), items.end());

  m_directoryCache.UpdateDirectory(path, items);
}

void CJustABunchOfFiles::OnAdd(const ADDON::CVFSDirEntry& item)
{
  if (!item.IsFolder())
  {
    CButtonMap* resource = CreateResource(item.Path());
    m_resources.AddResource(resource);
  }
}

void CJustABunchOfFiles::OnRemove(const ADDON::CVFSDirEntry& item)
{
  m_resources.RemoveResource(item.Path());
}

bool CJustABunchOfFiles::GetResourcePath(const CDevice& deviceInfo, std::string& resourcePath) const
{
  std::string strProviderFolder = m_strResourcePath + "/" + deviceInfo.Provider();

  // Ensure provider path exists
  if (CStorageUtils::EnsureDirectoryExists(strProviderFolder))
  {
    resourcePath = strProviderFolder + "/" + deviceInfo.RootFileName() + m_strExtension;
    return true;
  }

  return false;
}
