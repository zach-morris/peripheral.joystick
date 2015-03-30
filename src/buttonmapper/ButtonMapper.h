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
#pragma once

#include "kodi/threads/mutex.h"
#include "kodi/kodi_peripheral_utils.hpp"

#include <string>
#include <vector>

namespace JOYSTICK
{
  class CButtonMapper
  {
  private:
    CButtonMapper(void) { }

  public:
    static CButtonMapper& Get(void);

    virtual ~CButtonMapper(void) { Deinitialize(); }

    bool Initialize(void) { return true; }
    void Deinitialize(void) { }

    bool GetFeatures(const ADDON::Joystick& joystick, const std::string& strDevice,
                     std::vector<ADDON::JoystickFeature*>& features);

    bool MapFeature(const ADDON::Joystick& joystick, const std::string& strDevice,
                    ADDON::JoystickFeature* feature);
  };
}
