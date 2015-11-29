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

#include "storage/generic/Database.h"

#include "platform/threads/mutex.h"

#include <string>
#include <vector>

class TiXmlElement;

namespace JOYSTICK
{
  class CDatabaseXml : public CDatabase
  {
  public:
    CDatabaseXml(const std::string& strBasePath, bool bReadOnly);

    virtual ~CDatabaseXml(void) { }

    virtual bool GetFeatures(const CDevice& driverInfo, const std::string& controllerId,
                             FeatureVector& features) override;

    virtual bool MapFeature(const CDevice& driverInfo, const std::string& controllerId,
                            const ADDON::JoystickFeature& feature) override;

  private:
    bool Load(const CDevice& driverInfo);
    bool Save(void) const;

    bool Serialize(TiXmlElement* pElement) const;

    bool LoadButtonMaps(const std::string& strXmlPath);
    bool SaveButtonMaps(const CDevice& driverRecord, const std::string& strPath) const;

    bool SerializeButtonMaps(const CDevice& driverRecord, TiXmlElement* pElement) const;

    std::string      m_strDataPath;
    bool             m_bReadOnly;
    bool             m_bLoadAttempted;
    bool             m_bLoaded;
    PLATFORM::CMutex m_mutex;
  };
}
