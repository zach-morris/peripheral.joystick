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

#include "kodi/kodi_peripheral_utils.hpp"

#include <map>
#include <string>

namespace JOYSTICK
{
  class CButtonMap
  {
  public:
    CButtonMap(void) { }
    CButtonMap(const CButtonMap& other) { *this = other; }
    virtual ~CButtonMap(void) { Reset(); }

    CButtonMap& operator=(const CButtonMap& rhs);

    void Reset(void);

    bool IsEmpty(void) const { return m_buttons.empty(); }

    void GetFeatures(std::vector<ADDON::JoystickFeature*>& features) const;

    bool MapFeature(const ADDON::JoystickFeature* feature);

  protected:
    typedef std::string                                    FeatureName;
    typedef std::map<FeatureName, ADDON::JoystickFeature*> Buttons;

    Buttons m_buttons;

  private:
    bool UnmapPrimitive(const ADDON::DriverPrimitive& primitive);

    // Helper function
    ADDON::DriverPrimitive Opposite(const ADDON::DriverPrimitive& semiaxis);
  };
}
