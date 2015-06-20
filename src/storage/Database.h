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

#include "Device.h"

#include "kodi/kodi_peripheral_utils.hpp"

namespace JOYSTICK
{
  class CDatabase
  {
  public:
    virtual ~CDatabase(void) { }

    virtual bool GetFeatures(const CDevice& needle, const std::string& strDeviceId,
                             std::vector<ADDON::JoystickFeature*>& features);

    virtual bool MapFeature(const CDevice& needle, const std::string& strDeviceId,
                            const ADDON::JoystickFeature* feature);

  protected:
    std::vector<CDevice> m_devices;
  };
}
