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
#pragma once

#include "ButtonMap.h"
#include "Device.h"
#include "IDatabase.h"
#include "filesystem/DirectoryCache.h"

#include "p8-platform/threads/mutex.h"

#include <map>
#include <string>

namespace JOYSTICK
{
  class CResources
  {
  public:
    CResources(void) { }
    ~CResources(void);

    CButtonMap* GetResource(const CDevice& deviceInfo);
    bool AddResource(CButtonMap* resource);
    void RemoveResource(const std::string& strPath);

  private:
    typedef std::map<CDevice, CButtonMap*> ResourceMap;

    ResourceMap m_resources;
  };

  class CJustABunchOfFiles : public IDatabase,
                             public IDirectoryCacheCallback
  {
  public:
    CJustABunchOfFiles(const std::string& strResourcePath, const std::string& strExtension, bool bReadWrite);
    virtual ~CJustABunchOfFiles(void);

    // implementation of IDatabase
    virtual bool GetFeatures(const CDevice& driverInfo,
                             const std::string& controllerId,
                             FeatureVector& features) override;
    virtual bool MapFeatures(const CDevice& driverInfo,
                             const std::string& controllerId,
                             const FeatureVector& features) override;
    virtual bool ResetButtonMap(const CDevice& driverInfo,
                                const std::string& controllerId) override;

    // implementation of IDirectoryCacheCallback
    virtual void OnAdd(const ADDON::CVFSDirEntry& item) override;
    virtual void OnRemove(const ADDON::CVFSDirEntry& item) override;

  protected:
    virtual CButtonMap* CreateResource(const std::string& resourcePath) = 0;
    virtual CButtonMap* CreateResource(const std::string& resourcePath, const CDevice& driverInfo) = 0;

  private:
    void IndexDirectory(const std::string& path, unsigned int folderDepth);

    void AddResource(const std::string& strPath, CButtonMap* resource);

    bool GetResourcePath(const CDevice& deviceInfo, std::string& resourcePath) const;

    const std::string m_strResourcePath;
    const std::string m_strExtension;
    const bool        m_bReadWrite;
    CDirectoryCache   m_directoryCache;
    CResources        m_resources;
    P8PLATFORM::CMutex  m_mutex;
  };
}
