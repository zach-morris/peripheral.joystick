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

#include "ButtonMapTypes.h"

#include "kodi/kodi_peripheral_utils.hpp"
#include "platform/threads/mutex.h"

namespace JOYSTICK
{
  class CButtonMap
  {
  public:
    CButtonMap(void) { }

    CButtonMap& operator=(CButtonMap&& rhs);

    bool IsEmpty(void) const;

    size_t FeatureCount(void) const;

    void GetFeatures(FeatureVector& features) const;

    bool MapFeature(const FeaturePtr& feature);

  private:
    bool UnmapFeature(const FeaturePtr& feature);
    bool UnmapPrimitive(const ADDON::DriverPrimitive& primitive);

    // Helper function
    ADDON::DriverPrimitive Opposite(const ADDON::DriverPrimitive& semiaxis);

    FeatureVector            m_buttonMap;
    mutable PLATFORM::CMutex m_mutex;
  };
}
