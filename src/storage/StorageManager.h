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

#include "ButtonMapTypes.h"

#include "kodi/kodi_peripheral_types.h"
#include "kodi/kodi_peripheral_utils.hpp"

#include <string>
#include <vector>

class TiXmlElement;

namespace ADDON { class CHelper_libKODI_peripheral; }

namespace JOYSTICK
{
  class CDatabase;

  class CStorageManager
  {
  private:
    CStorageManager(void);

  public:
    static CStorageManager& Get(void);

    ~CStorageManager(void) { Deinitialize(); }

    bool Initialize(ADDON::CHelper_libKODI_peripheral* peripheralLib,
                    const PERIPHERAL_PROPERTIES& props);

    void Deinitialize(void);

    bool GetFeatures(const ADDON::Joystick& joystick, const std::string& strDeviceId,
                     FeatureVector& features);

    bool MapFeature(const ADDON::Joystick& joystick, const std::string& strDeviceId,
                    const ADDON::JoystickFeature* feature);

    void RefreshButtonMaps(const std::string& strDeviceName = "");

  private:
    ADDON::CHelper_libKODI_peripheral* m_peripheralLib;
    std::vector<CDatabase*>            m_databases;
  };
}
