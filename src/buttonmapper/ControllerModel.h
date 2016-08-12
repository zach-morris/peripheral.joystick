/*
 *      Copyright (C) 2016 Garrett Brown
 *      Copyright (C) 2016 Team Kodi
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
 *  along with this Program; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#pragma once

#include "ButtonMapTypes.h"

namespace JOYSTICK
{
  class CControllerModel
  {
  public:
    void Reset();

    ControllerMap& GetMap() { return m_map; }
    const ControllerMap& GetMap() const { return m_map; }

    const FeatureOccurrences& GetNormalizedFeatures(const ControllerMapItem& needle, bool bSwap);

  private:
    void Normalize(const ControllerMapItem& needle, bool bSwap);

    static void NormalizeFeatures(const FeatureOccurrences& feature, FeatureOccurrences& result, bool bSwap);

    ControllerMap m_map;
    ControllerMap m_reducedMap;
  };
}
