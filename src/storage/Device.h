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
#pragma once

#include "ButtonMap.h"

#include "kodi/kodi_peripheral_utils.hpp"

#include <map>
#include <stdint.h>
#include <string>

namespace JOYSTICK
{
  class CDevice
  {
  public:
    CDevice(void);
    CDevice(const ADDON::Joystick& joystick);

    virtual ~CDevice(void) { }

    void Reset(void) { *this = CDevice(); }

    bool operator==(const CDevice& rhs) const;

    bool IsValid(void) const;
    bool IsEmpty(void) const { return m_buttonMaps.empty(); }

    const std::string& Name(void) const { return m_strName; }

    bool GetFeatures(const std::string& strDeviceId, std::vector<ADDON::JoystickFeature*>& features) const;

    bool MapFeature(const std::string& strDeviceId, const ADDON::JoystickFeature* feature);

    bool MergeButtonMaps(const CDevice& device);

  protected:
    std::string  m_strName;
    std::string  m_strProvider;
    uint16_t     m_vid;
    uint16_t     m_pid;
    unsigned int m_buttonCount;
    unsigned int m_hatCount;
    unsigned int m_axisCount;

    typedef std::string                        ControllerID;
    typedef std::map<ControllerID, CButtonMap> ButtonMaps;

    ButtonMaps   m_buttonMaps;
  };
}
